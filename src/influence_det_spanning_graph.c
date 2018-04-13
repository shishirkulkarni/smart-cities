#include <smart-cities/libsc.h>
#include <igraph.h>
#include <stdlib.h>
#include <time.h>


#define PAGERANK "pagerank"
#define THRESHOLD "threshold"
#define WEIGHT "weight"

static void fill_vid_vector(igraph_t *g, igraph_vector_t *vids) {
	igraph_vit_t vit;
	igraph_vit_create(g, igraph_vss_all(), &vit);

	while(!IGRAPH_VIT_END(vit)) {
		igraph_vector_push_back(vids, IGRAPH_VIT_GET(vit));
		IGRAPH_VIT_NEXT(vit);
	}

	igraph_vit_destroy(&vit);
}

static void compute_threshold(igraph_t *g) {
	int n = igraph_vcount(g);
	//seed the random number generator
	srand(time(0));
	igraph_vit_t vit;

	igraph_vit_create(g, igraph_vss_all(), &vit);

	while(!IGRAPH_VIT_END(vit)) {
		SETVAN(g, THRESHOLD, IGRAPH_VIT_GET(vit), sc_rand_double());
		IGRAPH_VIT_NEXT(vit);
	}

	igraph_vit_destroy(&vit);
}

static void convert_to_directed(igraph_t *orig, igraph_t *dg) {
	igraph_copy(dg, orig);

	if(igraph_is_directed(orig)) {
		return;
	}

	igraph_to_directed(dg, IGRAPH_TO_DIRECTED_MUTUAL);
}

//edge influence based on degree as weight
static void compute_weight(igraph_t *g) {
	igraph_eit_t eit;
	igraph_eit_create(g, igraph_ess_all(IGRAPH_EDGEORDER_ID), &eit);

	while(!IGRAPH_EIT_END(eit)) {
		igraph_vector_t res;
		igraph_integer_t from, to;

		igraph_vector_init(&res, 1);

		igraph_edge(g, IGRAPH_EIT_GET(eit), &from, &to);

		igraph_degree(g, &res, igraph_vss_1(to), IGRAPH_IN, 0);

		SETEAN(g, WEIGHT, IGRAPH_EIT_GET(eit), VECTOR(res)[0] == 0 ? 0 : 1 / VECTOR(res)[0]);

		igraph_vector_destroy(&res);
		
		IGRAPH_EIT_NEXT(eit);
	}

	igraph_eit_destroy(&eit);
}

static void compute_pagerank(igraph_t *g, igraph_vector_t *result,
	igraph_real_t *max_pagerank_vertex) {
	
	igraph_real_t value = 0; //Eigenvalue of the result

	igraph_vector_init(result, 0);
	
	//daping factor and epsilon hardcoded for now
	igraph_pagerank_power_options_t options = {1000, 0.000001}; 
	
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


/*
* computes a spanning graph from a given graph with a given starting vertex
* @param orig_graph: The original graph
* @param span_graph: An uninitialized graph object
* @param begin_vertex: The beginning vertex of the spanning graph
*/

static void compute_acyclic_spanning_graph(igraph_t *orig_graph,
	igraph_t *span_graph, long int begin_vertex) {

	// add all the vertices to the spanning graph
	igraph_empty(span_graph, igraph_vcount(orig_graph),
		igraph_is_directed(orig_graph));

	// A lookup table to maintain visited vertices
	igraph_vector_t visited;
	igraph_dqueue_t q;

	igraph_vector_init(&visited, igraph_vcount(orig_graph));

	//none visited in the beginning
	igraph_vector_fill(&visited, 0.0);

	igraph_dqueue_init(&q, igraph_vcount(orig_graph));

	//Push root in the queue and visit it
	igraph_dqueue_push(&q, begin_vertex);
	VECTOR(visited)[(int)begin_vertex] = !VECTOR(visited)[(int)begin_vertex];

	// A vector to hold edges
	igraph_vector_t edges;
	igraph_vector_init(&edges, 0);

	while(!igraph_dqueue_empty(&q)) {
		
		igraph_real_t from = igraph_dqueue_pop(&q);

		//Visit popped vertex
		if(!VECTOR(visited)[(int)from])
			VECTOR(visited)[(int)from] = !VECTOR(visited)[(int)from];

		igraph_vs_t vs;
		igraph_vs_adj(&vs, (igraph_integer_t)from, IGRAPH_ALL);

		//extract centrality attribute
		igraph_matrix_t weights;
		sc_fill_matrix_attributes_vs(orig_graph, &weights, "pagerank", &vs);

		// sort matrix on the descending order of centrality measure, here
		// pagerank
		sc_sort_matrix(&weights, 1, SC_DESC);
		int i = 0;

		while(i < NROW(weights)) {
			igraph_real_t to = MATRIX(weights, i, 0);
			//if current vertex is not visited
			if(!VECTOR(visited)[(int)to]) {
				
				// visit the vertex
				VECTOR(visited)[(int)to] = !VECTOR(visited)[(int)to];

				// push the vertex in the queue
				igraph_dqueue_push(&q, to);

				// add the edge to graph
				igraph_vector_push_back(&edges, from);
				igraph_vector_push_back(&edges, to);
			}

			i++;
		}

		igraph_vs_destroy(&vs);
		igraph_matrix_destroy(&weights);
	}

	// add the edge to graph
	igraph_add_edges(span_graph, &edges, 0);

	igraph_vector_destroy(&visited);
	igraph_dqueue_destroy(&q);
}

/*
* function get_k_influencers: returns a vector containing top k influencers
 							  based on given centrality attribute.
* @param g: original graph with the centrality attribute set
* @param vec: pointer to an uninitialized vector. It will contain the 
			  influencer set
* @param centrality_attr: the centrality attribute set on the vertex.
						 eg. pagerank
* @param k: the number of influencers to extract
*/
//TODO: handle erroneous conditions
static void get_k_influencers(igraph_t *g, igraph_vector_t *vec,
	const char *centrality_attr, int k) {

	igraph_matrix_t mat;
	int i;

	const char *att[] =  {centrality_attr, NULL};
	
	sc_fill_matrix_attributes(g, &mat, att, SC_VERTEX);
	sc_sort_matrix(&mat, 1, SC_DESC);

	igraph_vector_init(vec, 0);

	for(i = 0; i < k; i++) {
		igraph_vector_push_back(vec, MATRIX(mat, i, 0));
	}

}

/* function: compute_current_diffusion updates the active set to new seed set
*/
static void compute_curr_diffusion(igraph_t *g, int k,
	igraph_vector_t *active_set, igraph_real_t vertex_threshold) {

	igraph_vector_t cur_active_list;

	igraph_vector_init(&cur_active_list, 0);
	igraph_vector_update(&cur_active_list, active_set);

	int n = igraph_vector_size(active_set);

	int i = 0;
	for(i = 0; i < n; i++) {

		// for every neighbour,
		igraph_vs_t vs;
		igraph_vit_t vit;
		igraph_vs_adj(&vs, VECTOR(*active_set)[i], IGRAPH_ALL);
		igraph_vit_create(g, vs, &vit);

		while(!IGRAPH_VIT_END(vit)) {

			igraph_real_t cur_vertex = IGRAPH_VIT_GET(vit);

			igraph_es_t es;
			igraph_es_adj(&es, cur_vertex, IGRAPH_IN);

			igraph_eit_t eit;
			igraph_eit_create(g, es, &eit);

			igraph_real_t sum = 0;

			while(!IGRAPH_EIT_END(eit)) {

				igraph_integer_t src, dest;
				igraph_edge(g, IGRAPH_EIT_GET(eit), &src, &dest);
				if(igraph_vector_contains(active_set, (igraph_integer_t)src))
					sum += EAN(g, WEIGHT, IGRAPH_EIT_GET(eit));
				IGRAPH_EIT_NEXT(eit);
			}


			if(sum > VAN(g, THRESHOLD, cur_vertex) && !igraph_vector_contains(&cur_active_list, cur_vertex)) {
				igraph_vector_push_back(&cur_active_list, cur_vertex);
			}

			igraph_eit_destroy(&eit);
			igraph_es_destroy(&es);

			IGRAPH_VIT_NEXT(vit);
		}

		igraph_vit_destroy(&vit);
		igraph_vs_destroy(&vs);

	}

	// copy current active list to seed set
	igraph_vector_update(active_set, &cur_active_list);
}



/* function compute_k_diffusion: computes the set of influenced vertices from
								a given seed set and a given threshold value
* @param g: graph
* @param k: size of the original seed set
* @param seed_set: The seed set obtained from the spanning graph algorithm
* @param vertex_threshold: Threshold value for the weight parameter
* @param total_k_influenced: uninitialized vector. The set of influnenced
						vertices from the given seed set.
* @param wt_attr: The weight attribute
*/ 
static void compute_k_diffusion(igraph_t *g, int k,
	igraph_vector_t *seed_set, igraph_real_t vertex_threshold,
	igraph_vector_t *total_k_influenced) {

	igraph_vector_t total_influenced, curr_active_nodes;

	igraph_vector_init(total_k_influenced, 0);
	igraph_vector_init(&curr_active_nodes, 0);
	igraph_vector_init(&total_influenced, 0);

	igraph_vector_update(&total_influenced, seed_set);
	igraph_vector_update(&curr_active_nodes, seed_set);

	while(1) {

		compute_curr_diffusion(g, k, &curr_active_nodes, vertex_threshold);
		
		if(igraph_vector_size(&curr_active_nodes) == igraph_vector_size(&total_influenced))
			break;

		igraph_vector_update(&total_influenced, &curr_active_nodes);
	}

	igraph_vector_update(total_k_influenced, &curr_active_nodes);
}

void get_hardcoded_seedset(igraph_vector_t *seed_set) {
	igraph_vector_init(seed_set, 0);
	igraph_vector_push_back(seed_set, 0);
	igraph_vector_push_back(seed_set, 33);
	igraph_vector_push_back(seed_set, 32);
}


int main(int argc, char *argv[]) {
	if(argc != 2) {
		printf("Usage: %s <input graph file> <seed set percentage>\n", argv[0]);
		exit(1);
	}

	// percentage of influencers
	// float k = 3;
	// k = (float)atoi(argv[2]) / 100;

	igraph_i_set_attribute_table(&igraph_cattribute_table);
	
	FILE *f = fopen(argv[1], "r");
	if(f == 0) {
		printf("Cannot open %s\n", argv[1]);
		exit(1);
	}

	igraph_t g, dg;
	igraph_read_graph_gml(&g, f);

	//create a directed graph from given graph
	convert_to_directed(&g, &dg);

	compute_threshold(&dg); //compute threshole only once

	// sc_print_vertex_attribute(&dg, THRESHOLD);

	// sc_print_edge_attribute(&dg, THRESHOLD);

	// return 0;


	int i;
	float k = 0.3;

	// for(k = 0.3; k <= 0.3; k += 0.05) {

		char filename[200];
		sprintf(filename, "observations/karate/influence_greedy/%d.txt", (int)(k * 100));

		FILE *fp = fopen(filename, "w");
		
		for(i = 0; i < 20; i++) {

			//Compute seed set from spanning graph
			igraph_vector_t pagerank;
			igraph_real_t max_pagerank_vertex;

			compute_pagerank(&g, &pagerank, &max_pagerank_vertex);

			// igraph_vector_t influencers;
			// get_k_influencers(&g, &influencers, PAGERANK, igraph_vcount(&g));

			igraph_t spanning_graph;
			compute_acyclic_spanning_graph(&g, &spanning_graph, max_pagerank_vertex);

			// Compute pagerank for spanning graph
			igraph_real_t span_max_pagerank_vertex;
			igraph_vector_t span_pagerank;

			compute_pagerank(&spanning_graph, &span_pagerank, &span_max_pagerank_vertex);


			igraph_vector_t seed_set;
			// get_k_influencers(&spanning_graph, &seed_set, PAGERANK, k * igraph_vcount(&spanning_graph));
			get_hardcoded_seedset(&seed_set);

			//end seed set computation

			// Use the linear threshold to compute for spread estimation
			compute_weight(&dg);

			igraph_vector_t k_influenced;
			compute_k_diffusion(&dg, k * igraph_vcount(&dg), &seed_set, 0.1, &k_influenced);

			fprintf(fp, "%f\t%f\t%d\n", k, (float) igraph_vector_size(&k_influenced) / igraph_vcount(&dg), igraph_vcount(&dg));
		}
	// }

	return 0;
}
