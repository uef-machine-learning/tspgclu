/*******************************************************************************
 *
 * This file is part of TSPG software.
 * Copyright (C) 2015-2018 Sami Sieranoja
 * <samisi@uef.fi>, <sami.sieranoja@gmail.com>
 *
 * TSPG is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version. You should have received a copy
 * of the GNU Lesser General Public License along with RKNNG.
 * If not, see <http://www.gnu.org/licenses/lgpl.html>.
 *******************************************************************************/
#include <stdio.h>
#include "contrib/argtable3.h"
#include <sys/mman.h> // mmap, munmap

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <vector>

// inline float dist33(int i, int j) {
float __attribute__((noinline)) dist33(int i, int j) { return i * j - 3.2; }
// float dist33(int i, int j) { return i * j - 3.2; }

// #define EXPERIMENTAL 1

using namespace std;

struct stat {
  int num_calc_clu_dist;
  int num_pruned;
};

int g_use_heap = 0;
struct stat g_stat;

void print_stat() {
  printf("STAT num_calc_clu_dist=%d num_pruned=%d\n", g_stat.num_calc_clu_dist, g_stat.num_pruned);
}

#include "tspg_lib.hpp"

kNNGraph *g_ground_truth;
#include "recall.h"

float distf(int i, int j) { return i * j - 20.0; }

int write_output_pa(int *part, int size, arg_file *outfn, int numClusters,
                    int output_write_header) {
  if (outfn->count > 0) {

    FILE *fp = fopen(outfn->filename[0], "w");
    if (fp == NULL) {
      perror("Can't write to file");
      return 1;
    }

    if (output_write_header) {
      fprintf(fp, "VQ PARTITIONING 2.0\n");
      fprintf(fp, "%d\n%d\n", numClusters, size);
      fprintf(fp, "-------------------------------------\n");
    }

    printf("Writing output to file: %s\n", outfn->filename[0]);
    write_ints_to_fp(fp, part, size);
    // write_ints_to_file(outfn->filename[0], part, data->size);
    fclose(fp);
  }
  return 0;
}

int main(int argc, char *argv[]) {

  signal(SIGSEGV, handler);
  setbuf(stdout, NULL); // Disable buffering on stdout

  double *distmat = NULL;

  g_stat.num_calc_clu_dist = 0;

  struct arg_dbl *distpar;
  struct arg_dbl *knng_start_nndes;
  struct arg_dbl *stopDelta;
  struct arg_end *end;
  struct arg_file *gtfn;
  struct arg_file *infn;
  struct arg_file *a_graphfn;
  struct arg_file *a_centroidfn;
  struct arg_file *a_mergeorderfn;
  struct arg_file *mapfn;
  struct arg_file *outfn;
  struct arg_int *a_costf;
  struct arg_int *a_maxn;
  struct arg_int *a_minn;
  struct arg_int *a_ndist_est;
  struct arg_int *a_numClusters;
  struct arg_int *a_numtsp;
  struct arg_int *a_prune;
  struct arg_int *a_riter;
  struct arg_int *a_tlimit;
  struct arg_int *a_samples;
  struct arg_int *a_uncle;
  struct arg_int *a_scale_method;
  struct arg_int *a_verbose;
  struct arg_int *bfsize;
  struct arg_int *maxIterations;
  struct arg_int *nthreads;
  struct arg_int *numNeighbors;
  struct arg_int *rngSeed;
  struct arg_lit *a_header;
  struct arg_lit *help;
  struct arg_lit *a_meanc;
  struct arg_str *algo;
  struct arg_str *distfunc;
  struct arg_str *dtype;
  struct arg_str *informat;
  struct arg_str *outf;
  struct arg_str *a_gtype;

  // printf("ED:%d\m",edit_distance("abc","abd"));

  float start_nndes = 0.1; // For RP-div knng algorithm
  int output_write_header = 0;
  DataSet *data = NULL;
  // int data_type = T_NUMERICAL;
  tspg::datatype dtyp = tspg::NUMERICAL;
  kNNGraph *kNN;

  g_options.costf = 5;
  g_options.distance_type = 0;
  g_options.max_neighbors = 20;
  g_options.min_neighbors = 2;
  g_options.neighbor_dist_estimation = 0;
  g_options.num_samples = 50;
  g_options.prune_strategy = 0;
  g_options.refine_iter = 0;
  g_options.scale_method = 2;
  g_options.time_limit = 0;
  g_options.verbose = 0;
  g_options.gtype = RPDIV;
  g_options.mean_calculation = 1;

  g_ground_truth = NULL;
  linkedList *ll;

  void *argtable[] = {
      help = arg_litn(NULL, "help", 0, 1, "display this help and exit"),

      distpar = arg_dbln(NULL, "distpar", "<FLOAT>", 0, 1,
                         "Parameter to distance function (minkowski p-value)"),

      a_numClusters = arg_intn("C", "numclu", "<n>", 0, 1, "Number of clusters"),
      a_header = arg_litn("H", "output-header", 0, 1, "Include header info in output partition"),

      a_scale_method =
          arg_intn(NULL, "scale", "<n>", 0, 1, "Scale method {1=no scaling (default), 2=x^2}"),

      a_costf = arg_intn(NULL, "costf", "<n>", 0, 1,
                         "Costf function {1=totalc,2=compl,3=aver,4=single,5=tse}"),
      a_verbose = arg_intn("V", "verbose", "<n>", 0, 1, "Verbose level {0,1,2}"),
      a_numtsp = arg_intn("T", "numtsp", "<n>", 0, 1, "Number of TSP paths in the graph"),

      rngSeed = arg_intn(NULL, "seed", "<n>", 0, 1, "random number seed"),

      a_samples = arg_intn(NULL, "samples", "<n>", 0, 1,
                           "Number of samples in distance estimation. 0 for no sampling."),

      dtype =
          arg_str0(NULL, "type", "<vec|txt|set>", "Input data type: vectorial, text or set data."),
      distfunc = arg_str0(NULL, "dfunc", "<FUNC>",
                          "Distance function:\n"
                          "     l2 = euclidean distance (vectorial, default)\n"
                          // "     mnkw = Minkowski distance (vectorial)\n" TODO
                          "     lev = Levenshtein distance (for strings, default)\n"),
      // "     dice = Dice coefficient / bigrams (for strings)\n"),

      a_gtype = arg_str0(NULL, "gtype", "<type>", "Graph type: {rpdiv (default), compl (slow)}"),
      algo = arg_str0(NULL, "algo", "<name>",
                      "Algorithm: {tspg,tspgclu}\n"
                      "     tspg = Create the TSP-grap\n"
                      "     tspgclu = Agglomerative clustering using the TSP-graph (default)\n"),

      outfn = arg_filen("o", "out", "<file>", 0, 1, "output partition file"),
      a_centroidfn = arg_filen(NULL, "cfn", "<file>", 0, 1, "output centroids filename"),
      a_mergeorderfn =
          arg_filen(NULL, "mergefn", "<file>", 0, 1, "output merge order filename (TODO)"),
      infn = arg_filen(NULL, NULL, "<file>", 1, 1, "input files"),

      a_graphfn = arg_filen(NULL, "graphfn", "<file>", 0, 1, "graph used for distance calculation"),
      a_meanc = arg_litn(NULL, "meanc", 0, 1, "Use mean vector calculation for numerical data"),

#ifdef EXPERIMENTAL
      mapfn = arg_filen(NULL, "mapfn", "<file>", 0, 1, "file for one-dimensional mappings"),
      informat =
          arg_str0(NULL, "format", "<ascii|lshkit>", "Input format: ascii or lshkit (binary)"),
      outf = arg_str0(NULL, "outf", "<format>", "Output format: {txt,ivec,wgraph}"),

      bfsize = arg_intn("W", "bfsize", "<n>", 0, 1, "divide until size <= W"),
      numNeighbors = arg_intn("k", "num_neighbors", "<n>", 0, 1, "number of neighbors"),
      maxIterations = arg_intn("I", "maxIter", "<STOP>", 0, 1, "Iterations > STOP "),
      stopDelta = arg_dbln(NULL, "delta", "<STOP>", 0, 1, "Stop when delta < STOP "),
      a_riter = arg_intn("r", "riter", "<n>", 0, 1, "Number of refine iterations"),

      a_minn = arg_intn(NULL, "minn", "<n>", 0, 1, "Minimum number of neighbors in graph"),
      a_maxn = arg_intn(NULL, "maxn", "<n>", 0, 1, "Maximum number of neighbors in graph"),
      a_prune = arg_intn(NULL, "prune", "<n>", 0, 1, "Pruning strategy {0,1}"),
      gtfn = arg_filen(NULL, "gt", "<file>", 0, 1, "Ground truth graph file (ivec)"),

      knng_start_nndes =
          arg_dbln(NULL, "knng-nndes", "START", 0, 1, "Start using nndes when delta < START"),
      nthreads = arg_intn(NULL, "threads", "<n>", 0, 1, "Number of threads"),

      a_uncle = arg_intn(NULL, "exp-uncle", "<n>", 0, 1, "Experimental"),
      a_ndist_est = arg_intn(NULL, "ndist-est", "<n>", 0, 1, "Experimental"),

      a_tlimit = arg_intn(NULL, "tlimit", "<n>", 0, 1, "Exit after n seconds (debug)"),
#endif

      end = arg_end(20),
  };

  int ok = 1;
  int nerrors = arg_parse(argc, argv, argtable);
  // if(nerrors > 0) {terminal_error("Unable to parse command line\n");}
  if (nerrors > 0) {
    ok = 0;
  }

  double delta = 0.01;
  int K = 0;
  int W;
  int maxIter = 100;
  int numClusters = 15;

  if (a_scale_method->count > 0) {
    g_options.scale_method = a_scale_method->ival[0];
  }

  if (a_costf->count > 0) {
    g_options.costf = a_costf->ival[0];
  }

  if (g_options.costf == TSE) {
    g_options.scale_method = 2;
  }

  if (a_verbose->count > 0) {
    g_options.verbose = a_verbose->ival[0];
  }

  if (a_header->count > 0) {
    output_write_header = 1;
  }

  int seed;
  if (rngSeed->count > 0) {
    seed = rngSeed->ival[0];
  } else {
    seed = time(NULL);
  }

  srand(seed);
  printf("RNG seed: %d\n", seed);

  // if (numNeighbors->count > 0) {
    // K = numNeighbors->ival[0];
  // }
  // W = 2.5 * K;

  if (a_numClusters->count > 0) {
    numClusters = a_numClusters->ival[0];
  }

  if (infn->count > 0) {
  } else {
    printf("No infile\n");
    ok = 0;
  }

  if (dtype->count > 0 && strcmp(dtype->sval[0], "txt") == 0) {
    dtyp = tspg::STRING;
  } else if (dtype->count > 0 && strcmp(dtype->sval[0], "vec") == 0) {
    dtyp = tspg::NUMERICAL;
  } else if (dtype->count > 0 && strcmp(dtype->sval[0], "set") == 0) {
    dtyp = tspg::SET;
  } else {
    printf("Must specify data type: vec|txt|set\n");
    ok = 0;
  }

  g_options.gtype = RPDIV;
  if (a_gtype->count > 0 && strcmp(a_gtype->sval[0], "compl") == 0) {
    g_options.gtype = COMPL;
  }

  if (help->count > 0 || ok == 0) {
    printf("Clustering using TPS-graph (v. 0.1).\n\ntspg");
    arg_print_syntax(stdout, argtable, "\n");
    arg_print_glossary(stdout, argtable, "  %-25s %s\n");
    return 0;
  }

  // TODO:
  // if (data_type == T_SET) {
  // data = loadSetData(infn->filename[0]);
  // g_options.mean_calculation = 0;
  // } else {
  // terminal_error("Incorrect data type\n");
  // }
  // printf("data->type %d\n", data->type);

  g_options.num_tsp = 1;
  if (a_numtsp->count > 0) {
    g_options.num_tsp = a_numtsp->ival[0];
  }

  if (distpar->count > 0) {
    g_options.minkowski_p = distpar->dval[0];
  } else {
    g_options.minkowski_p = 1.0; // L1 = Manhattan distance
  }

  if (a_samples->count > 0) {
    g_options.num_samples = a_samples->ival[0];
  }
  printf("Samples: %d\n", g_options.num_samples);

#ifdef EXPERIMENTAL

  if (outf->count > 0 && strcmp(outf->sval[0], "txt") == 0) {
    printf("Output format:txt\n");
  }

  if (maxIterations->count > 0) {
    maxIter = maxIterations->ival[0];
  }

  if (a_prune->count > 0) {
    g_options.prune_strategy = a_prune->ival[0];
  }
  if (stopDelta->count > 0) {
    delta = stopDelta->dval[0];
  }

  g_options.uncle_adjustment = 3;
  if (a_uncle->count > 0) {
    g_options.uncle_adjustment = a_uncle->ival[0];
  }

  if (a_ndist_est->count > 0) {
    g_options.neighbor_dist_estimation = a_ndist_est->ival[0];
  }

  if (a_tlimit->count > 0) {
    g_options.time_limit = a_tlimit->ival[0];
  }

  if (bfsize->count > 0) {
    W = bfsize->ival[0];
  }

  if (knng_start_nndes->count > 0) {
    start_nndes = (float)knng_start_nndes->dval[0];
  }

  if (a_riter->count > 0) {
    g_options.refine_iter = a_riter->ival[0];
  }

  if (a_minn->count > 0) {
    g_options.min_neighbors = a_minn->ival[0];
  }

  if (a_maxn->count > 0) {
    g_options.max_neighbors = a_maxn->ival[0];
  }

  if (nthreads->count > 0) {
    printf("Threads: %d\n", nthreads->ival[0]);
    g_options.num_threads = nthreads->ival[0];
  } else {
    g_options.num_threads = 1;
  }

  g_options.recall_K = K;
#endif
  tspg::Distance *dfun = nullptr;

  // if (data_type == T_NUMERICAL) {
  if (dtyp == tspg::NUMERICAL) {
    data = read_ascii_dataset(infn->filename[0]);

    if (a_meanc->count > 0) {
      g_options.mean_calculation = 1;
    }

    // Default distance function L2
    dfun = dynamic_cast<tspg::Distance *>(new tspg::L2df(data));

    if (distfunc->count > 0) {
      if (strcmp(distfunc->sval[0], "l2") == 0) {
        printf("Distance function: %s\n", distfunc->sval[0]);
        g_options.mean_calculation = 1;
      } else if (strcmp(distfunc->sval[0], "l1") == 0) {
        printf("Distance function: %s\n", distfunc->sval[0]);
        dfun = dynamic_cast<tspg::Distance *>(new tspg::L1df(data));
      }
    }
  }

  if (dtyp == tspg::STRING) {

    printf("STRING data\n");
    data = loadStringData(infn->filename[0]);
    g_options.mean_calculation = 0;

    if (distfunc->count > 0) {
      dfun = dynamic_cast<tspg::Distance *>(new tspg::EditDistance(data));
      if (strcmp(distfunc->sval[0], "lev") == 0) {
        printf("Distance function: Levenshtein\n");
      } else {
        terminal_error("Unknown distance function\n");
      }
    }
  }

  // if (distfunc->count > 0) {

  // else if (strcmp(distfunc->sval[0], "lev") == 0) {

  // TODO:
  // else if (strcmp(distfunc->sval[0], "mnkw") == 0) {
  // g_options.distance_type = DIST_MINKW;
  // printf("Distance function: minkowski (p=%f)\n", g_options.minkowski_p);
  // } else if (strcmp(distfunc->sval[0], "cos") == 0) {
  // printf("Distance function: Cosine\n");
  // g_options.distance_type = DIST_COS;
  // } else if (strcmp(distfunc->sval[0], "dice") == 0) {
  // g_options.distance_type = DIST_DICE;
  // printf("Distance function: Dice\n");
  // }

  // else {
  // terminal_error("Unknown distance function\n");
  // }
  // }

  printf("numtsp=%d costf=%d samples=%d distance_type=%d infn='%s'\n", g_options.num_tsp,
         g_options.costf, g_options.num_samples, g_options.distance_type, infn->filename[0]);

  // Start counting time
  g_timer.tick();

  nnGraph *graph = NULL;
  // Using graph based distance function
  if (a_graphfn->count >= 1) {
    // int seed = time(NULL);
    int seed = 1677674001;
    srand(seed);
    printf("RNGg seed: %d\n", seed);

    std::map<std::string, int> *cdmap;
    graph = read_ascii_graphf2(a_graphfn->filename[0], 1, &cdmap);
    // printf("#2#F43:%d\n", (*cdmap)[string("F43")]);
    g_timer.tick();
    data = loadSetDataWithMapping(infn->filename[0], cdmap);
    tspg::GraphDistance *graphd = new tspg::GraphDistance(graph, data, cdmap->size());
    printf("graphd size: %d\n", graphd->size);

    TSPclu<tspg::Distance> tspgclu(numClusters /*K=clusters*/, g_options.num_tsp /*num_tsp*/,
                                   dynamic_cast<tspg::Distance *>(graphd),
                                   false /*mean calculation*/);

    int *part = tspgclu.runClustering();
    dealloc_nnGraph(graph);
    if (outfn->count > 0) {
      write_output_pa(part, data->size, outfn, numClusters, 1);
    }

    delete cdmap;

    return 0;
  }

  // Cluster using the TSP-graph
  if (algo->count > 0 && strcmp(algo->sval[0], "tspgclu") == 0) {
    printf("Algorithm: TSPg-clu\n");

    nnGraph *nng = init_nnGraph(data->size);

    printf("dfun size: %d\n", dfun->size);

    TSPclu<tspg::Distance> tspgclu(numClusters /*K=clusters*/, g_options.num_tsp /*num_tsp*/, dfun,
                                   g_options.mean_calculation);

    printf("nng stat ");
    graph_stat(nng);
    printf("\n");

    vector<vector<float>> *centroids;
    centroids = NULL;
    if (g_options.mean_calculation) {
      centroids = new vector<vector<float>>(numClusters, vector<float>(data->dimensionality, 0));
    }

    // TODO: return centroids
    int *part = tspgclu.runClustering();

    // if (g_options.mean_calculation && a_centroidfn->count >= 1) {
    // write_flt_vec2_to_file(a_centroidfn->filename[0], centroids);
    // }

    if (outfn->count > 0) {
      write_output_pa(part, data->size, outfn, numClusters, output_write_header);
    }
  }

  return 0;

  // Create the TSP-graph
  if (algo->count > 0 && strcmp(algo->sval[0], "tspg") == 0) {
    printf("Algorithm: TSPg\n");
    nnGraph *g = init_nnGraph(data->size);
    // nnGraph *nng = create_tspg(data, g, K, W, delta, start_nndes, g_options.num_tsp, &ll);
    nnGraph *nng = create_tspg(data, g, g_options.num_tsp, &ll);
    if (outfn->count > 0) {
      printf("Writing output to file: %s\n", outfn->filename[0]);
      write_nngraph_to_file(nng, outfn->filename[0]);
    }
    return 0;
  }

  return 0;
}
