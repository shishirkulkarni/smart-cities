#include <igraph.h>
#include <stdlib.h>
#include <smart-cities/libsc.h>

#define NOVER "nover"

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: bin/louvain <input graph file>\n");
		exit(1);
	}

	igraph_t g;
	FILE *fp;

	igraph_i_set_attribute_table(&igraph_cattribute_table);

	fp = fopen(argv[1], "r");
	
	if(fp == 0) {
		printf("Error opening file\n");
		exit(0);
	}

	igraph_vector_t weights, membership;
	igraph_vector_init(&weights, 0);
	igraph_vector_init(&membership, 0);


	igraph_read_graph_gml(&g, fp);
	sc_calculate_nover(&g, NOVER);
	sc_fill_vector_edge_nattribute(&g, &weights, NOVER);

	int i = 0;

	for(i = 0; i < 20; i++) {
		igraph_community_multilevel(&g, &weights, &membership, NULL, NULL);

		igraph_real_t modularity;
		igraph_modularity(&g, &membership, &modularity, &weights);

		printf("Average modularity: %f\n", modularity);		
	}
}