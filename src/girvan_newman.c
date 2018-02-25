#include <igraph.h>
#include <stdlib.h>
#include <smart-cities/libsc.h>

int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: bin/girvan_newman <input graph file>\n");
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
	
	igraph_read_graph_gml(&g, fp);

	igraph_vector_t edge_betweenness,
		modularity,
		membership;
	igraph_vector_init(&edge_betweenness, 0);
	igraph_vector_init(&modularity, 0);
	igraph_vector_init(&membership, 0);

	int i = 0;

	for(i = 0; i < 20; i++) {
		igraph_community_edge_betweenness(&g, NULL, &edge_betweenness, NULL, NULL, &modularity, &membership, 0, NULL);
		printf("Average modularity: %f\n", igraph_vector_max(&modularity));
	}

	return 0;
}