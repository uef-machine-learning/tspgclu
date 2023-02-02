
#ifndef KNNG_OPTIONS_H
#define KNNG_OPTIONS_H

#include <stdlib.h>

struct knng_options {

    double minkowski_p;
    int costf; // Cost function
    int distance_type;
    int gtype;
    int max_neighbors;
    int mean_calculation;
    int min_neighbors;
    int neighbor_dist_estimation;
    int nndes_K;
    int num_samples;
    int num_threads;
    int num_tsp;
    int prune_strategy;
    int recall_K;
    int refine_graph;
    int refine_iter;
    int scale_method;
    int time_limit;
    int uncle_adjustment;
    int verbose;
};




#endif
