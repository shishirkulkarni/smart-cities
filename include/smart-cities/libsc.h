#include <igraph.h>

#ifndef _SMART_CITIES_LIB_H
#define _SMART_CITIES_LIB_H


#define SC_RANDOM_FILE "/dev/urandom"
int sc_rand(unsigned int);

#define SC_ASC 0
#define SC_DESC 1

#define NROW(m) igraph_matrix_nrow((&m))
#define NCOL(m) igraph_matrix_ncol((&m))

void sc_sort_matrix(igraph_matrix_t*, int, int);

void sc_print_matrix(igraph_matrix_t);

void sc_print_graph(igraph_t);

void sc_print_vector(igraph_vector_t);

void sc_fill_vector_edge_nattribute(igraph_t*, igraph_vector_t*, const char*);

void sc_mst_kruskal(igraph_t*, igraph_t*, igraph_bool_t);


// Union find data structure
typedef struct sc_union_find {
	long unsigned *nodes;
	long unsigned *parent;
	unsigned n;
} sc_union_find;

sc_union_find* sc_union_find_init_igraph(igraph_t*);
void sc_union_find_destroy(sc_union_find*);
void sc_print_union_find(sc_union_find*);
long int sc_union_find_find(sc_union_find*, int);
void sc_union_find_union(sc_union_find*, int, int);
void sc_mst_kruskal_igraph(igraph_t*, igraph_t*, const char*);

#endif