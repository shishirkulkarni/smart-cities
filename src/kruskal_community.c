#include <igraph.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <smart-cities/libsc.h>

#define PRIM 0
#define KRUSKAL 1
#define BORUVKA 2

#define NOVER "nover"
#define EDGE_BETWEENNESS "eb"

#define HERE printf("Here\n")

/*
* Currently assuming that vertex iterators and edge iterators are going to return
* the values in sorted order only. So the computation of neighborhood overlap
* becomes easier.
*/

static void calculate_nover_for_edges(igraph_t *g, const char *att_name) {
	int retval, i;
	igraph_es_t es;
	igraph_eit_t eit;

	// DO NOT CHANGE THIS. THE CALCULATION OF COMMON VERTICES IS
	// BASED ON THE ASSUMPTION THAT EDGES AND VERTICES ARE SORTED
	retval = igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
	assert(retval == 0);
	retval = igraph_eit_create(g, es, &eit);
	assert(retval == 0);

	while(!IGRAPH_EIT_END(eit)) {
		long int edge = IGRAPH_EIT_GET(eit);
		int u = IGRAPH_FROM(g, edge), v = IGRAPH_TO(g, edge),
			common_count = 0, total_count = 0;
		
		igraph_vs_t u_adj, v_adj;
		igraph_vit_t u_vit, v_vit;
		igraph_vs_adj(&u_adj, u, IGRAPH_ALL);
		igraph_vs_adj(&v_adj, v, IGRAPH_ALL);
		igraph_vit_create(g, u_adj, &u_vit);
		igraph_vit_create(g, v_adj, &v_vit);

		/*
		* Using the logic of merging 2 sorted lists to count common elements
		*/
		while(!IGRAPH_VIT_END(u_vit) && !IGRAPH_VIT_END(v_vit)) {
			int u = IGRAPH_VIT_GET(u_vit), v = IGRAPH_VIT_GET(v_vit);
			if(u == v) {
				common_count++;
				total_count++;
				IGRAPH_VIT_NEXT(u_vit);
				IGRAPH_VIT_NEXT(v_vit);
			}
			else if (u < v) {
				IGRAPH_VIT_NEXT(u_vit);
				total_count++;
			} else {
				IGRAPH_VIT_NEXT(v_vit);
				total_count++;
			}
		}

		while(!IGRAPH_VIT_END(u_vit)) {
			total_count++;
			IGRAPH_VIT_NEXT(u_vit);
		}

		while(!IGRAPH_VIT_END(v_vit)) {
			total_count++;
			IGRAPH_VIT_NEXT(v_vit);
		}

		//set NOVER as the edge's weight
		SETEAN(g, att_name, edge, (double)common_count / (total_count - 2));

		igraph_vit_destroy(&u_vit);
		igraph_vit_destroy(&v_vit);

		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
	igraph_es_destroy(&es);
}

static void compute_mst(igraph_t *graph, igraph_t *tree, int algo, const char* wt_attr) {
	igraph_vector_t weights, res;
	igraph_eit_t eit;

	switch(algo) {
		default:
		case PRIM:
			igraph_vector_init(&weights, 0);
			igraph_eit_create(graph, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

			while(!IGRAPH_EIT_END(eit)) {
				igraph_vector_push_back(&weights, EAN(graph, wt_attr, IGRAPH_EIT_GET(eit)));
				IGRAPH_EIT_NEXT(eit);
			}

			igraph_eit_destroy(&eit);
			igraph_minimum_spanning_tree_prim(graph, tree, &weights);
			break;
		case KRUSKAL:
			sc_mst_kruskal_igraph(graph, tree, wt_attr);
			break;
		case BORUVKA:
			sc_mst_boruvka_igraph(graph, tree, wt_attr);
			break;
	}
}


/*
* @param tree: graph containing the minimum spanning tree
* @param att_name: name of the attribute to set on graph
*/
static void compute_edge_betweenness(igraph_t *g, const char *att_name) {
	int i = 0;

	igraph_vector_t eb;

	igraph_vector_init(&eb, igraph_ecount(g));

	igraph_edge_betweenness(g, &eb, IGRAPH_UNDIRECTED, 0);

	igraph_es_t es;
	igraph_eit_t eit;

	igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
	igraph_eit_create(g, es, &eit);

	while(!IGRAPH_EIT_END(eit)) {
		SETEAN(g, att_name, IGRAPH_EIT_GET(eit), VECTOR(eb)[i++]);
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_vector_destroy(&eb);

	igraph_es_destroy(&es);
	igraph_eit_destroy(&eit);
}

static void print_attributes_n(igraph_t *g, const char *attr_name) {
	igraph_es_t es;
	igraph_eit_t eit;
	int retval;

	retval = igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
	assert(retval == 0);
	retval = igraph_eit_create(g, es, &eit);
	assert(retval == 0);

	while(!IGRAPH_EIT_END(eit)) {
		printf("%s: %f\n", attr_name, EAN(g, attr_name, IGRAPH_EIT_GET(eit)));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}


/*
*	
*/
static void compute_modularity_k(igraph_t *tree, igraph_matrix_t lookup,
			int k, igraph_real_t *modularity, const char *wt_attr) {
	int i;
	igraph_t copy_tree;
	igraph_copy(&copy_tree, tree);


	igraph_vector_t v;
	igraph_vector_init(&v, 0);

	for(i = 0; i < k; i++) {
		igraph_vector_push_back(&v, (int)MATRIX(lookup, i, 0));
	}

	// sc_print_vector(v);

	igraph_delete_edges(&copy_tree, igraph_ess_vector(&v));
	// printf("%d\n", igraph_ecount(&copy_tree));

	igraph_vector_t membership, csize, weights;
	int count;

	igraph_vector_init(&membership, 0);
	igraph_vector_init(&csize, 0);
	igraph_vector_init(&weights, 0);

	igraph_clusters(&copy_tree, &membership, &csize, &count, IGRAPH_STRONG);

	sc_fill_vector_edge_nattribute(&copy_tree, &weights, wt_attr);
	
	igraph_modularity(&copy_tree, &membership, modularity, &weights);
}

static void label_vertices(igraph_t *g) {
	igraph_vit_t vit;

	igraph_vit_create(g, igraph_vss_all(), &vit);

	while(!IGRAPH_VIT_END(vit)) {
		SETVAN(g, "label", IGRAPH_VIT_GET(vit), IGRAPH_VIT_GET(vit));
		IGRAPH_VIT_NEXT(vit);
	}

	igraph_vit_destroy(&vit);
}

static void label_edges(igraph_t *g) {
	igraph_eit_t eit;
	igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

	while(!IGRAPH_EIT_END(eit)) {
		SETEAN(g, "label", IGRAPH_EIT_GET(eit), IGRAPH_EIT_GET(eit));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}

igraph_bool_t validate_mst(igraph_t *tree) {
	igraph_integer_t connected_components_strong, connected_components_weak;
	igraph_clusters(tree, NULL, NULL, &connected_components_strong, IGRAPH_STRONG);
	igraph_clusters(tree, NULL, NULL, &connected_components_weak, IGRAPH_WEAK);
	printf("Strong: %d\n", connected_components_strong);
	printf("Weak: %d\n", connected_components_weak);
	printf("Ecount: %d, Vcount: %d\n", igraph_ecount(tree), igraph_vcount(tree));
	return connected_components_strong == 1 && connected_components_weak == 1
		&& igraph_ecount(tree) == igraph_vcount(tree) - 1;
}


static void print_attributes(igraph_t *g) {
	igraph_strvector_t names;
	igraph_strvector_init(&names, 0);

	igraph_cattribute_list(g, NULL, NULL, NULL, NULL, &names, NULL);

	sc_print_strvector(names);
}


int main(int argc, char *argv[]) {
	igraph_t g;
	FILE *ifile;
	long int i;

	/* turn on attribute handling */
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	ifile=fopen(argv[1], "r");
	if (ifile == 0) {
		return 10;
	}

	igraph_read_graph_gml(&g, ifile);
	fclose(ifile);

	// print_attributes(&g);

	//Set numeric labels for edges and vertices
	// label_vertices(&g);
	// label_edges(&g);

	// Calculate Neighborhood overlap
	calculate_nover_for_edges(&g, NOVER);

	// Edge betweenness before	
	igraph_vector_t eb;
	compute_edge_betweenness(&g, EDGE_BETWEENNESS);

	igraph_t tree;
	// compute_mst(&g, &tree, KRUSKAL, NOVER);
	compute_mst(&g, &tree, BORUVKA, NOVER);


	// Edge betweenness after	
	// igraph_vector_t eb;
	// compute_edge_betweenness(&tree, EDGE_BETWEENNESS);

	igraph_es_t es;
	igraph_eit_t eit;
	igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
	igraph_eit_create(&tree, es, &eit);

	// Create a lookup matrix to remove k-1 edges based on descending order
	// of edge betweenness
	igraph_matrix_t lookup;
	igraph_matrix_init(&lookup, igraph_ecount(&tree), 2);
	
	while(!IGRAPH_EIT_END(eit)) {
		long int e_id = IGRAPH_EIT_GET(eit); 
		MATRIX(lookup, IGRAPH_EIT_GET(eit), 0) = IGRAPH_EIT_GET(eit);
		MATRIX(lookup, IGRAPH_EIT_GET(eit), 1) = EAN(&tree, EDGE_BETWEENNESS, IGRAPH_EIT_GET(eit));
		IGRAPH_EIT_NEXT(eit);
	}

	sc_sort_matrix(&lookup, 1, SC_DESC);

	//After this, everytime we will generate a random number k,
	// Copy the mst to a new variable and delete k-1 edges from the
	// new varaible and compute its modularity
	igraph_vector_t visited;

	igraph_vector_init(&visited, 0);

	igraph_real_t approx_modularity = 0.0;

	for(i = 0; i < 20 && igraph_vector_size(&visited) <= igraph_ecount(&tree); i++) {

		int k = sc_rand(igraph_matrix_nrow(&lookup));

		//Generate a non visited random number
		while(igraph_vector_contains(&visited, k)) {
			k = sc_rand(igraph_matrix_nrow(&lookup));
		}

		igraph_vector_push_back(&visited, k);

		igraph_real_t modularity_k = -1,
			modularity_k_2 = -1,
			modularity_2k = -1;

		igraph_real_t sum = 0.0;
		int count = 0; 
		
		compute_modularity_k(&tree, lookup, k, &modularity_k, EDGE_BETWEENNESS);
		sum += modularity_k;
		count++;

		compute_modularity_k(&tree, lookup, k / 2, &modularity_k_2, EDGE_BETWEENNESS);
		sum += modularity_k_2;
		count++;

		if(2 * k < igraph_ecount(&tree)) {
			compute_modularity_k(&tree, lookup, 2 * k, &modularity_2k, EDGE_BETWEENNESS);
			sum += modularity_2k;
			count++;
		}
		
		sum = sum / count;
		approx_modularity += sum;
		
	}


	printf("Approx Modularity: %f\n", approx_modularity / igraph_vector_size(&visited));
	
	return 0;
}
