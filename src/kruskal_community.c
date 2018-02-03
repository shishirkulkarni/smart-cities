#include <igraph.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include <smart-cities/lib.h>

/*
* Currently assuming that vertex iterators and edge iterators are going to return
* the values in sorted order only. So the computation of neighborhood overlap
* becomes easier.
*/

static void calculate_nover_for_edges(igraph_t *g) {
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
		SETEAN(g, "weight", edge, (double)common_count / (total_count - 2));

		igraph_vit_destroy(&u_vit);
		igraph_vit_destroy(&v_vit);

		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
	igraph_es_destroy(&es);
}

//Currently computing the spanning tree using Prim's Algorithm
// provided by the library
static void compute_mst(igraph_t *graph, igraph_t *tree) {
	igraph_vector_t weights, res;
	int i, retval;

	igraph_vector_init(&weights, igraph_ecount(graph));
	igraph_vector_init(&res, igraph_ecount(graph));

	igraph_minimum_spanning_tree_prim(graph, tree, &weights);
}


/*
* @param tree: graph containing the minimum spanning tree
* @param eb: _uninitialized_ igraph vector
*/
static void compute_edge_betweenness(igraph_t *tree, igraph_vector_t *eb) {
	int i = 0;

	igraph_vector_init(eb, igraph_ecount(tree));

	igraph_edge_betweenness(tree, eb, IGRAPH_UNDIRECTED, 0);

	igraph_es_t es;
	igraph_eit_t eit;

	igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
	igraph_eit_create(tree, es, &eit);

	while(!IGRAPH_EIT_END(eit)) {
		SETEAN(tree, "eb", IGRAPH_EIT_GET(eit), VECTOR(*eb)[i++]);
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_es_destroy(&es);
	igraph_eit_destroy(&eit);
}

static void print_weights(igraph_t *g) {
	igraph_es_t es;
	igraph_eit_t eit;
	int retval;

	retval = igraph_es_all(&es, IGRAPH_EDGEORDER_ID);
	assert(retval == 0);
	retval = igraph_eit_create(g, es, &eit);
	assert(retval == 0);

	while(!IGRAPH_EIT_END(eit)) {
		printf("Weight: %f\n", EAN(g, "weight", IGRAPH_EIT_GET(eit)));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}



int main() {
	igraph_t g;
	FILE *ifile;
	// igraph_vector_t gtypes, vtypes, etypes;
	// igraph_strvector_t gnames, vnames, enames;
	long int i;
	// igraph_vector_t y;
	// igraph_strvector_t id;
	// char str[20];

	/* turn on attribute handling */
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	ifile=fopen("data/karate.gml", "r");
	if (ifile==0) {
		return 10;
	}

	igraph_read_graph_gml(&g, ifile);
	fclose(ifile);

	// igraph_vector_init(&gtypes, 0);
	// igraph_vector_init(&vtypes, 0);
	// igraph_vector_init(&etypes, 0);
	// igraph_strvector_init(&gnames, 0);
	// igraph_strvector_init(&vnames, 0);
	// igraph_strvector_init(&enames, 0);
  
	// igraph_cattribute_list(&g, &gnames, &gtypes, &vnames, &vtypes, 
		// &enames, &etypes);

	// Set Labels to vertices
	for(i = 0; i < igraph_vcount(&g); i++) {
		SETVAN(&g, "label", i, i);
	}
	// Set Labels to edges
	for(i = 0; i < igraph_ecount(&g); i++) {
		SETEAN(&g, "label", i, i);
	}

	// Calculate Neighborhood overlap
	calculate_nover_for_edges(&g);
	
	// Calculate edge betweenness
	igraph_vector_t eb;
	compute_edge_betweenness(&g, &eb);
	
	igraph_t tree;
	compute_mst(&g, &tree);
	

	// Compute modularity and local maximum
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
		MATRIX(lookup, IGRAPH_EIT_GET(eit), 1) = EAN(&tree, "eb", IGRAPH_EIT_GET(eit));
		IGRAPH_EIT_NEXT(eit);
	}

	sc_sort_matrix(&lookup, 1, SC_DESC);
	// sc_print_matrix(lookup);

	//After this, everytime you will generate a random number k,
	// Copy the mst to a new variable and delete k-1 edges from the
	// new varaible and compute its modularity

	igraph_t copy_tree;
	int k = sc_rand(igraph_matrix_nrow(&lookup));
	
	igraph_copy(&copy_tree, &tree);

	igraph_vector_t v;
	igraph_vector_init(&v, 0);

	for(i = 0; i < k; i++) {
		igraph_vector_push_back(&v, (int)MATRIX(lookup, i, 0));
	}

	igraph_delete_edges(&copy_tree, igraph_ess_vector(&v));

	igraph_vector_t membership, csize, weights;
	int count;
	igraph_real_t modularity;

	igraph_vector_init(&membership, 0);
	igraph_vector_init(&csize, 0);
	igraph_vector_init(&weights, 0);

	igraph_clusters(&copy_tree, &membership, &csize, &count, IGRAPH_STRONG);

	sc_fill_vector_edge_nattribute(&copy_tree, &weights, "eb");
	
	igraph_modularity(&copy_tree, &membership, &modularity, &weights);

	printf("modularity: %f\n", modularity);

	return 0;
}
