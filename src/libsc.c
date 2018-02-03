#include <smart-cities/lib.h>
#include <stdio.h>
#include <stdlib.h>

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


void sc_fill_vector_edge_nattribute(igraph_t *g, igraph_vector_t *v, const char *att_name) {
	igraph_eit_t eit;

	igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

	while(!IGRAPH_EIT_END(eit)) {
		igraph_vector_push_back(v, EAN(g, att_name, IGRAPH_EIT_GET(eit)));
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}

