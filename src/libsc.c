#include <smart-cities/libsc.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>


/*
* Union find API
* TODO: implement path compression
*/

sc_union_find* sc_union_find_init_igraph(igraph_t *g) {
	int i;

	sc_union_find *uf = (sc_union_find*) malloc(sizeof(sc_union_find));
	uf->nodes = (long unsigned*) malloc(igraph_vcount(g) * sizeof(long unsigned));
	uf->parent = (long unsigned*) malloc(igraph_vcount(g) * sizeof(long unsigned));
	uf->rank = (long unsigned*) malloc(igraph_vcount(g) * sizeof(long unsigned));
	uf->n = igraph_vcount(g);

	igraph_vit_t vit;
	igraph_vit_create(g, igraph_vss_all(), &vit);

	while(!IGRAPH_VIT_END(vit)) {
		uf->nodes[IGRAPH_VIT_GET(vit)] = uf->parent[IGRAPH_VIT_GET(vit)] = IGRAPH_VIT_GET(vit);
		uf->rank[IGRAPH_VIT_GET(vit)] = 0;
		IGRAPH_VIT_NEXT(vit);
	}

	igraph_vit_destroy(&vit);
	return uf;
}


void sc_union_find_destroy(sc_union_find *uf) {
	free(uf->nodes);
	free(uf->parent);
	free(uf->rank);
}

void sc_print_union_find(sc_union_find *uf) {
	int i;
	for(i = 0; i < uf->n; i++) {
		printf("{%ld, %ld, %ld} ", uf->nodes[i], uf->parent[i], uf->rank[i]);
	}
	printf("\n");
}

long int sc_union_find_find(sc_union_find *uf, int vertex_id) { 
	if(vertex_id >= uf->n || vertex_id < 0) {
		printf("*******************Union find error*******************\n");
		return -1;
	}

	if(vertex_id != uf->parent[vertex_id])
		uf->parent[vertex_id] = sc_union_find_find(uf, uf->parent[vertex_id]);

	return uf->parent[vertex_id];
}


//TODO: implement path compression
void sc_union_find_union(sc_union_find *uf, int src, int dest) {
	long int src_root = sc_union_find_find(uf, src),
		dest_root = sc_union_find_find(uf, dest);

	if(src_root == -1 || dest_root == -1) {
		printf("*******************Union find error*******************\n");
		return;
	}

	if(uf->rank[src_root] < uf->rank[dest_root]) {
		uf->parent[src_root] = dest_root;
	} else if(uf->rank[src_root] > uf->rank[dest_root]) {
		uf->parent[dest_root] = src_root;
	} else {
		uf->parent[dest_root] = src_root;
		uf->rank[src_root]++;
	}

	// uf->parent[src_find] = uf->parent[dest_find];
}


/* API ends here */
int sc_rand(unsigned int max) {
	FILE *f = fopen(SC_RANDOM_FILE, "r");
	int random_no;
	fread(&random_no, sizeof(random_no), 1, f);
	fclose(f);
	if(max == 0) {
		return random_no;
	}
	
	return random_no % max;
}


/*
* A generic matrix sorter based on column_index
* implementing bubble_sort for now.
* TODO: implement a better sorting algorithm
*/
void sc_sort_matrix(igraph_matrix_t *m, int key_col, int order) {
	long int rows = igraph_matrix_nrow(m),
		cols = igraph_matrix_ncol(m);
	long int i, j;
	for(i = 0; i < rows - 1; i++) {
		for(j = 0; j < rows - 1 - i; j++) {
			if(order == SC_ASC) {
				if(MATRIX(*m, j, key_col) > MATRIX(*m, j + 1, key_col)) {
					igraph_matrix_swap_rows(m, j, j + 1);
				}
			} else {
				if(MATRIX(*m, j, key_col) < MATRIX(*m, j + 1, key_col)) {
					igraph_matrix_swap_rows(m, j, j + 1);
				}
			}
		}
	}
}

void sc_print_matrix(igraph_matrix_t m) {
	int i, j;
	for(i = 0; i < igraph_matrix_nrow(&m); i++) {
		for(j = 0; j < igraph_matrix_ncol(&m); j++) {
			printf("%f ", MATRIX(m, i, j));
		}
		printf("\n");
	}
}

void sc_print_graph(igraph_t g) {
	igraph_vit_t vit;
	printf("Vertices:\n");
	igraph_vit_create(&g, igraph_vss_all(), &vit);

	
	while(!IGRAPH_VIT_END(vit)) {
		printf("%d ", IGRAPH_VIT_GET(vit));
		IGRAPH_VIT_NEXT(vit);
	}

	igraph_vit_destroy(&vit);

	printf("\n");

	igraph_eit_t eit;
	igraph_eit_create(&g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

	printf("Edges:\n");
	
	while(!IGRAPH_EIT_END(eit)) {
		printf("%d ", (int)EAN(&g, "label", IGRAPH_EIT_GET(eit)));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);

	printf("\n");
}


void sc_print_vector(igraph_vector_t v) {
	int i = 0;
	for(i = 0; i < igraph_vector_size(&v); i++) {
		printf("%f ", VECTOR(v)[i]);
	}
	printf("\n");
}

void sc_print_strvector(igraph_strvector_t v) {
	int i = 0;
	for(i = 0; i < igraph_strvector_size(&v); i++) {
		printf("%s ", STR(v, i));
	}
	printf("\n");
}


void sc_fill_vector_edge_nattribute(igraph_t *g, igraph_vector_t *v, const char *att_name) {
	igraph_eit_t eit;

	igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

	while(!IGRAPH_EIT_END(eit)) {
		igraph_vector_push_back(v, EAN(g, att_name, IGRAPH_EIT_GET(eit)));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}


/*
* create a matrix from the attribute names given
* @param g: the original graph
* @param mat: uninitialized matrix_t object
* @param attr_names: null terminated array of strings containing attribute names
*/
void sc_fill_matrix_attributes(igraph_t *g, igraph_matrix_t *mat,
	const char *attr_names[], sc_attribute_type type) {
	int i, j;
	for(i = 0; attr_names[i] != NULL; i++);
	int n = i;

	switch(type) {
		case SC_VERTEX:
			igraph_matrix_init(mat, igraph_vcount(g), n + 1);
			igraph_vit_t vit;
			igraph_vit_create(g, igraph_vss_all(), &vit);
			while(!IGRAPH_VIT_END(vit)) {
				
				MATRIX(*mat, IGRAPH_VIT_GET(vit), 0) = IGRAPH_VIT_GET(vit);
				
				for(j = 0; j < n; j++) {
					MATRIX(*mat, IGRAPH_VIT_GET(vit), j + 1) = VAN(g, attr_names[j], IGRAPH_VIT_GET(vit));
				}

				IGRAPH_VIT_NEXT(vit);
			}

			igraph_vit_destroy(&vit);
			break;
		case SC_EDGE:
			igraph_matrix_init(mat, igraph_ecount(g), n + 1);
			igraph_eit_t eit;
			igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
			while(!IGRAPH_EIT_END(eit)) {

				MATRIX(*mat, IGRAPH_EIT_GET(eit), 0) = IGRAPH_EIT_GET(eit);
				
				for(j = 0; j < n; j++) {
					MATRIX(*mat, IGRAPH_EIT_GET(eit), j + 1) = EAN(g, attr_names[j], IGRAPH_EIT_GET(eit));
				}

				IGRAPH_EIT_NEXT(eit);
			}
			igraph_eit_destroy(&eit);
			break;
	}
}

// Only the weight attribute will be copied over to the tree edge
// TODO: implement copying of all attributes
void sc_mst_kruskal_igraph(igraph_t *g, igraph_t *tree, const char *wt_attr) {
	if(wt_attr == NULL)
		return;

	igraph_strvector_t names;
	igraph_strvector_init(&names, 0);

	igraph_cattribute_list(g, NULL, NULL, NULL, NULL, &names, NULL);

	// sc_print_strvector(names);

	int i, count;

	igraph_empty(tree, igraph_vcount(g), igraph_is_directed(g));


	igraph_matrix_t wt_matrix;
	igraph_matrix_init(&wt_matrix, igraph_ecount(g), 2);
	igraph_eit_t eit;
	igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);
	
	while(!IGRAPH_EIT_END(eit)) {
		MATRIX(wt_matrix, IGRAPH_EIT_GET(eit), 0) = IGRAPH_EIT_GET(eit);
		MATRIX(wt_matrix, IGRAPH_EIT_GET(eit), 1) = EAN(g, wt_attr, IGRAPH_EIT_GET(eit));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);

	sc_sort_matrix(&wt_matrix, 1, SC_ASC);
	
	sc_union_find *uf = sc_union_find_init_igraph(tree);
	
	for(i = 0, count = 0; i < NROW(wt_matrix); i++) {
		int from, to, from_find, to_find;
		igraph_edge(g, MATRIX(wt_matrix, i, 0), &from, &to);

		from_find = sc_union_find_find(uf, from);
		to_find = sc_union_find_find(uf, to);

		if(from_find != to_find) {
			igraph_add_edge(tree, from, to);
			sc_union_find_union(uf, from_find, to_find);
			int j;
			
			//Copy over all the attributes from the original edge
			for(j = 0; j < igraph_strvector_size(&names); j++) {
				SETEAN(tree, STR(names, j), count, EAN(g, STR(names, j), MATRIX(wt_matrix, i, 0)));
			}

			count++;
		}
	}

	igraph_strvector_destroy(&names);

	sc_union_find_destroy(uf);	
}


void sc_print_edge_attribute(igraph_t *g, const char *attr_name) {
	igraph_eit_t eit;
	igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

	while(!IGRAPH_EIT_END(eit)) {
		printf("eid: %d, %s: %f\n", IGRAPH_EIT_GET(eit), attr_name, EAN(g, attr_name, IGRAPH_EIT_GET(eit)));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}

void sc_calculate_nover(igraph_t *g, const char *attr_name) {
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
		SETEAN(g, attr_name, edge, (double)common_count / (total_count - 2));

		igraph_vit_destroy(&u_vit);
		igraph_vit_destroy(&v_vit);

		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
	igraph_es_destroy(&es);
}



