#include <igraph.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

// int print_attributes(const igraph_t *g) {

//   igraph_vector_t gtypes, vtypes, etypes;
//   igraph_strvector_t gnames, vnames, enames;
//   long int i;

//   igraph_vector_t vec;
//   igraph_strvector_t svec;
//   long int j;

//   igraph_vector_init(&gtypes, 0);
//   igraph_vector_init(&vtypes, 0);
//   igraph_vector_init(&etypes, 0);
//   igraph_strvector_init(&gnames, 0);
//   igraph_strvector_init(&vnames, 0);
//   igraph_strvector_init(&enames, 0);

//   igraph_cattribute_list(g, &gnames, &gtypes, &vnames, &vtypes, 
// 			 &enames, &etypes);

//   /* Graph attributes */
//   for (i=0; i<igraph_strvector_size(&gnames); i++) {
//     printf("%s=", STR(gnames, i));
//     if (VECTOR(gtypes)[i]==IGRAPH_ATTRIBUTE_NUMERIC) {
//       igraph_real_printf(GAN(g, STR(gnames,i)));
//       putchar(' ');
//     } else {
//       printf("\"%s\" ", GAS(g, STR(gnames,i)));
//     }
//   }
//   printf("\n");

//   for (i=0; i<igraph_vcount(g); i++) {
//     long int j;
//     printf("Vertex %li: ", i);
//     for (j=0; j<igraph_strvector_size(&vnames); j++) {
//       printf("%s=", STR(vnames, j));
//       if (VECTOR(vtypes)[j]==IGRAPH_ATTRIBUTE_NUMERIC) {
// 	igraph_real_printf(VAN(g, STR(vnames,j), i));
// 	putchar(' ');
//       } else {
// 	printf("\"%s\" ", VAS(g, STR(vnames,j), i));
//       }
//     }
//     printf("\n");
//   }

//   for (i=0; i<igraph_ecount(g); i++) {
//     long int j;
//     printf("Edge %li (%i-%i): ", i, (int)IGRAPH_FROM(g,i), (int)IGRAPH_TO(g,i));
//     for (j=0; j<igraph_strvector_size(&enames); j++) {
//       printf("%s=", STR(enames, j));
//       if (VECTOR(etypes)[j]==IGRAPH_ATTRIBUTE_NUMERIC) {
// 	igraph_real_printf(EAN(g, STR(enames, j), i));
// 	putchar(' ');
//       } else {
// 	printf("\"%s\" ", EAS(g, STR(enames, j), i));
//       }
//     }
//     printf("\n");
//   }

//    Check vector-based query functions 
//   igraph_vector_init(&vec, 0);
//   igraph_strvector_init(&svec, 0);
  
//   for (j=0; j<igraph_strvector_size(&vnames); j++) {
//     if (VECTOR(vtypes)[j]==IGRAPH_ATTRIBUTE_NUMERIC) {
//       igraph_cattribute_VANV(g, STR(vnames, j), igraph_vss_all(), &vec);
//       for (i=0; i<igraph_vcount(g); i++) {
// 	igraph_real_t num=VAN(g, STR(vnames, j), i);
// 	if (num != VECTOR(vec)[i] &&
// 	    (!isnan(num) || !isnan(VECTOR(vec)[i]))) {
// 	  exit(51);
// 	}
//       }
//     } else {
//       igraph_cattribute_VASV(g, STR(vnames, j), igraph_vss_all(), &svec);
//       for (i=0; i<igraph_vcount(g); i++) {
// 	const char *str=VAS(g, STR(vnames, j), i);
// 	if (strcmp(str,STR(svec, i))) {
// 	  exit(52);
// 	}
//       }
//     }
//   }

//   for (j=0; j<igraph_strvector_size(&enames); j++) {
//     if (VECTOR(etypes)[j]==IGRAPH_ATTRIBUTE_NUMERIC) {
//       igraph_cattribute_EANV(g, STR(enames, j), 
// 			     igraph_ess_all(IGRAPH_EDGEORDER_ID), &vec);
//       for (i=0; i<igraph_ecount(g); i++) {
// 	igraph_real_t num=EAN(g, STR(enames, j), i);
// 	if (num != VECTOR(vec)[i] && 
// 	    (!isnan(num) || !isnan(VECTOR(vec)[i]))) {
// 	  exit(53);
// 	}
//       }
//     } else {
//       igraph_cattribute_EASV(g, STR(enames, j), 
// 			     igraph_ess_all(IGRAPH_EDGEORDER_ID), &svec);
//       for (i=0; i<igraph_ecount(g); i++) {
// 	const char *str=EAS(g, STR(enames, j), i);
// 	if (strcmp(str,STR(svec, i))) {
// 	  exit(54);
// 	}
//       }
//     }
//   }

//   igraph_strvector_destroy(&svec);
//   igraph_vector_destroy(&vec);

//   igraph_strvector_destroy(&enames);
//   igraph_strvector_destroy(&vnames);
//   igraph_strvector_destroy(&gnames);
//   igraph_vector_destroy(&etypes);
//   igraph_vector_destroy(&vtypes);
//   igraph_vector_destroy(&gtypes);

//   return 0;
// }


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
	igraph_t g, g2;
	FILE *ifile;
	igraph_vector_t gtypes, vtypes, etypes;
	igraph_strvector_t gnames, vnames, enames;
	long int i;
	igraph_vector_t y;
	igraph_strvector_t id;
	char str[20];

	/* turn on attribute handling */
	igraph_i_set_attribute_table(&igraph_cattribute_table);

	ifile=fopen("data/karate.gml", "r");
	if (ifile==0) {
		return 10;
	}

	igraph_read_graph_gml(&g, ifile);
	fclose(ifile);

	igraph_vector_init(&gtypes, 0);
	igraph_vector_init(&vtypes, 0);
	igraph_vector_init(&etypes, 0);
	igraph_strvector_init(&gnames, 0);
	igraph_strvector_init(&vnames, 0);
	igraph_strvector_init(&enames, 0);
  
	igraph_cattribute_list(&g, &gnames, &gtypes, &vnames, &vtypes, 
		&enames, &etypes);

	// Set Labels to vertices
	for(i = 0; i < igraph_vcount(&g); i++) {
		SETVAN(&g, "label", i, i);
	}

	// Calculate Neighborhood overlap
	calculate_nover_for_edges(&g);
	// print_weights(&g);

	igraph_t tree;
	compute_mst(&g, &tree);
	return 0;
}
