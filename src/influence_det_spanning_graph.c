#include <smart-cities/libsc.h>
#include <igraph.h>
#include <stdlib.h>


static void fill_vid_vector(igraph_t *g, igraph_vector_t *vids) {
	igraph_vit_t vit;
	igraph_vit_create(g, igraph_vss_all(), &vit);

	while(!IGRAPH_VIT_END(vit)) {
		igraph_vector_push_back(vids, IGRAPH_VIT_GET(vit));
		IGRAPH_VIT_NEXT(vit);
	}

	igraph_vit_destroy(&vit);
}

static void compute_pagerank(igraph_t *g, igraph_vector_t *result, igraph_real_t *max_pagerank_vertex) {
	igraph_real_t value = 0; //Eigenvalue of the result

	igraph_vector_init(result, 0);

	igraph_pagerank_power_options_t options = {1000, 0.000001}; //hardcoded for now
	
	igraph_pagerank(g,
		IGRAPH_PAGERANK_ALGO_POWER,
		result,
		&value, 
		igraph_vss_all(),
		igraph_is_directed(g),
		0.5,
		NULL,
		&options);

	// calculate the vertex having max pagerank value
	int i;
	*((int *)max_pagerank_vertex) = 0;
	SETVAN(g, "pagerank", 0, VECTOR(*result)[0]);
	for(i = 1; i < igraph_vector_size(result); i++) {
		if(VECTOR(*result)[*((int *)max_pagerank_vertex)] < VECTOR(*result)[i])
			*max_pagerank_vertex = i;
		SETVAN(g, "pagerank", i, VECTOR(*result)[i]);
	}
}




int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s <input graph file>\n", argv[0]);
		exit(1);
	}

	igraph_i_set_attribute_table(&igraph_cattribute_table);
	
	FILE *f = fopen(argv[1], "r");
	if(f == 0) {
		printf("Cannot open %s\n", argv[1]);
		exit(1);
	}

	igraph_t g;
	igraph_read_graph_gml(&g, f);

	igraph_vector_t pagerank;
	igraph_real_t max_pagerank_vertex;

	compute_pagerank(&g, &pagerank, &max_pagerank_vertex);

	igraph_matrix_t mat;

	const char *att[] =  {"pagerank", NULL};
	sc_fill_matrix_attributes(&g, &mat, att, SC_VERTEX);

	sc_sort_matrix(&mat, 1, SC_DESC);
	sc_print_matrix(mat);
	
	return 0;
}