
#ifndef RANDOMTREE_SEARCH_H
#define RANDOMTREE_SEARCH_H

#include "nndes.h"

void rpdiv_recurse(DataSet *data, int *input_arr, int *input_arr2, int input_arr_size, int level,
                   kNNGraph *knng, int brute_force_size, int *update_count) {
  int *left_arr;
  int *right_arr;
  int *left_arr2;
  int *right_arr2;
  int left_size = 0;
  int right_size = 0;

  // Run brute force algorithm (conquer)
  if (input_arr_size < brute_force_size) {

    for (int i = 0; i < input_arr_size; i++) {
      int i_point = input_arr[i];
      for (int j = i + 1; j < input_arr_size; j++) {
        int i_other_point = input_arr[j];
        double _dist = distance(data, i_point, i_other_point);
        g_dist_count++;
        *update_count += updatekNN(knng, i_point, i_other_point, _dist);
        *update_count += updatekNN(knng, i_other_point, i_point, _dist);
      }
    }
    return;
  }
  // Else divide

  int randind_A = input_arr[(int)(floor(input_arr_size * RAND_FLOAT() - 0.000001))];
  int randind_B = input_arr[(int)(floor(input_arr_size * RAND_FLOAT() - 0.000001))];

  int class_A_count = 0;
  int class_B_count = 0;

  /*input_arr2[class_A_count]=randind_A;*/
  /*input_arr2[input_arr_size -1 -class_B_count]=randind_B;*/
  /*class_A_count++; class_B_count++;*/

  for (int i = 0; i < input_arr_size; i++) {
    double dist_A = distance(data, input_arr[i], randind_A);
    double dist_B = distance(data, input_arr[i], randind_B);

    if (dist_A < dist_B) {
      input_arr2[class_A_count] = input_arr[i];
      class_A_count++;
    } else {
      input_arr2[input_arr_size - 1 - class_B_count] = input_arr[i];
      class_B_count++;
    }
  }

  for (int i = 0; i < input_arr_size; i++) {
    input_arr[i] = input_arr2[i];
  }

  left_size = class_A_count;
  right_size = class_B_count;

  left_arr = input_arr2;
  right_arr = input_arr2 + class_A_count;

  left_arr2 = input_arr;
  right_arr2 = input_arr + class_A_count;

  if (left_size > 3) {
    rpdiv_recurse(data, left_arr, left_arr2, left_size, level + 1, knng, brute_force_size,
                  update_count);
  }
  if (right_size > 3) {
    rpdiv_recurse(data, right_arr, right_arr2, right_size, level + 1, knng, brute_force_size,
                  update_count);
  }
}

// Random pair divisive construction of kNN graph
// (also) implementation of NNDES when nndesStart=1.0 & window_width=0
// TODO: s/window_width/W/ ?

kNNGraph *rpdiv_create_knng(DataSet *data, DataSet *DS_proj, int K, int window_width, double delta,
                            double nndesStart, int maxIterations, linkedList **ll_ret) {
  kNNGraph *knng = NULL;
  DataSet *projDS = data;
  knng = init_kNNGraph(data->size, g_options.recall_K, K);
  int update_count = 0;
  double update_portion = 0.0;

  int nndes_max_k = K;
  int nndes_k = K;
  int k_increment = 2;
  int run_nndes = 0;
  double update_portion_nndes = 0;

  if (window_width != 0) {
    printf("Fast random pair divisive (RP-Div) construction of kNN graph v. 0.1\n");
  } else {
    printf("NNDES construction of kNN graph v. 0.1\n");
  }

  // Two copies of tree. Optimization to avoid memory alloc/dealloc in future steps
  int *ind_arr = (int *)safemalloc(sizeof(int) * data->size);
  int *ind_arr2 = (int *)safemalloc(sizeof(int) * data->size);

  printf("K=%d W=%d delta=%f nndes_start=%f data type %d, distance type:%d min_neighbors:%d "
         "max_neighbors:%d\n",
         K, window_width, delta, nndesStart, data->type, g_options.distance_type,
         g_options.min_neighbors, g_options.max_neighbors);

  knng->k = K; // TODO

  linkedList *ll = initLinkedList();
  /*ll_add_node(ll,(void*)  &a);*/
  /*ll_add_node(ll,(void*)  &b);*/
  /*ll_add_node(ll,(void*)  &c);*/

  // nnGraph *g = init_nnGraph(data->size);

  // Create initial random graph:
  for (int i_data = 0; i_data < data->size; i_data++) {
    for (int j = 0; j < K; j++) {
      int i_other_point = i_data;
      while (i_other_point == i_data) {
        i_other_point = rand_int(data->size - 1);
      }

      double _dist = distance(data, i_data, i_other_point);
      g_dist_count++;
      updatekNN(knng, i_data, i_other_point, _dist);
    }
  }

  if (window_width == 0 || nndesStart >= 1.0) {
    run_nndes = 1;
  }

  for (int i_iter = 0;; i_iter++) {
    printf("iter=%d ", i_iter);
    g_timer.tuck("time");
    update_count = 0;
    int update_count_nndes = 0;

    if (window_width > 0) {
      // Do Random point hierarchical subdivision
      for (int i_data = 0; i_data < data->size; i_data++) {
        ind_arr[i_data] = i_data;
      }
      rpdiv_recurse(projDS, ind_arr, ind_arr2, projDS->size, 0, knng, window_width, &update_count);

      /*FILE *fp;*/
      /*fp = fopen("1dmap.txt","a");*/
      /*for(int i=0;i<data->size;i++) {*/
      /*fprintf(fp,"%d\n",ind_arr[i]);*/
      /*}*/
      /*fclose(fp);*/

      update_portion = ((double)update_count) / ((double)(data->size * knng->k)); // TODO:??
      printf("RP-div update_count=%d changes=%f%%\n", update_count, update_portion * 100);

      // int *map = (int *)malloc(sizeof(int) * data->size);
      // for (int i_data = 0; i_data < data->size; i_data++) {
        // map[i_data] = ind_arr[i_data];
      // }
      // ll_add_node(ll, (void *)map);

      // for (int i_data = 0; i_data < data->size - 1; i_data++) {
        // int a, b;
        // a = ind_arr[i_data];
        // b = ind_arr[i_data + 1];
        // double _dist = distance(data, a, b);
        // nng_add_mutual_neighbor(g, a, b, _dist);
      // }
    }

    if (update_portion < nndesStart || run_nndes) {
      // Do NNDES
      run_nndes = 1;
      g_timer.tuck("start nndes");
      update_count_nndes = nndes_iterate_limited(data, knng, nndes_k);
      update_portion_nndes = ((double)update_count_nndes) / ((double)(data->size * K));
      printf("NNDES k=%d update_count_nndes=%d changes=%f%%\n", nndes_k, update_count_nndes,
             update_portion_nndes * 100);
      calculate_recall(knng);
      if (nndes_k < nndes_max_k) {
        nndes_k += k_increment;
        knng->k += k_increment;
      }
    }

    // TODO
    calculate_recall(knng);

    if (update_portion + update_portion_nndes < delta) {
      printf("Reached end condition: changes=%f%% < %f%%",
             (update_portion + update_portion_nndes) * 100, delta * 100);

      break;
    } else if (i_iter + 1 >= maxIterations) {
      printf("Reached end condition: iterations == %d", maxIterations);
      break;
    }
  }

  return knng;
}

#endif
