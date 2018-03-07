#include <igraph.h>

#ifndef _SMART_CITIES_LIB_H
#define _SMART_CITIES_LIB_H


#define SC_RANDOM_FILE "/dev/urandom"
int sc_rand();
float sc_rand_double();
#define SC_ASC 0
#define SC_DESC 1

typedef unsigned sc_attribute_type;
#define SC_VERTEX 0
#define SC_EDGE 1


#define NROW(m) igraph_matrix_nrow((&m))
#define NCOL(m) igraph_matrix_ncol((&m))

void sc_sort_matrix(igraph_matrix_t*, int, int);

void sc_print_matrix(igraph_matrix_t);

void sc_print_graph(igraph_t);

void sc_print_vector(igraph_vector_t);
void sc_print_strvector(igraph_strvector_t);

void sc_fill_vector_edge_nattribute(igraph_t*, igraph_vector_t*, const char*);

void sc_mst_kruskal(igraph_t*, igraph_t*, igraph_bool_t);


// Union find data structure
typedef struct sc_union_find {
	long unsigned *nodes;
	long unsigned *parent;
	long unsigned *rank;
	unsigned n;
} sc_union_find;

sc_union_find* sc_union_find_init_igraph(igraph_t*);
void sc_union_find_destroy(sc_union_find*);
void sc_print_union_find(sc_union_find*);
long int sc_union_find_find(sc_union_find*, int);
void sc_union_find_union(sc_union_find*, int, int);
void sc_mst_kruskal_igraph(igraph_t*, igraph_t*, const char*);

void sc_calculate_nover(igraph_t*, const char*);
void sc_print_edge_attribute(igraph_t*, const char*);
void sc_fill_matrix_attributes(igraph_t*, igraph_matrix_t*,
	const char **, sc_attribute_type);
void sc_fill_matrix_attributes_vs(igraph_t*, igraph_matrix_t*,
	const char*, igraph_vs_t*);

#endif