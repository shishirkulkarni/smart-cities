#include <igraph.h>
#include <stdio.h>


int main(void) {
	FILE *karate_gml;
	igraph_t g;
	karate_gml = fopen("data/karate.gml", "r");
	igraph_read_graph_gml(&g, karate_gml);

	igraph_vector_t order;
	igraph_vector_init(&order, 0);

	igraph_bfs(&g, 0, NULL, IGRAPH_ALL, 1, NULL, &order, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
	
	long int i;
	for(i = 0; i < igraph_vector_size(&order); i++) {
		printf(" %li\n", (long int) VECTOR(order)[i]);
	}

	return 0;
}