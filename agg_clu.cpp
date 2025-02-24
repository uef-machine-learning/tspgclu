
int CmpDistP(void *a, void *b, void *info) {
  float diff = (((gNode *)a)->nearest_dist - ((gNode *)b)->nearest_dist);
  return (diff > 0 ? 1 : diff == 0 ? 0 : -1);
}

void merge_graphs_knn(nnGraph *g, kNNGraph *kNN) {

  int idx = 0;
  int K = 0;
  K = kNN->k;

  // fprintf(fp,"%d\n",kNN->size);
  for (int data_i = 0; data_i < kNN->size; data_i++) {

    int idA = kNN->list[data_i].id;
    // fprintf(fp,"%u",kNN->list[data_i].id);
    // fprintf(fp," %d",K);

    // Neighbors
    for (int nn_i = 0; nn_i < kNN->k; nn_i++) {
      int idB = (int)get_kNN_item_id(kNN, data_i, nn_i);
      float dist = get_kNN_item_dist(kNN, data_i, nn_i);
      nng_add_mutual_neighbor2(g, idA, idB, dist);
      // fprintf(fp," %i",idx);
    }
    // Distances to neighbors
    // for(int nn_i = 0; nn_i < kNN->k; nn_i++) {
    // float dist = get_kNN_item_dist(kNN,data_i,nn_i);
    // fprintf(fp," %f",dist);
    // }
    // fprintf(fp,"\n");
  }
}

void graph_stat(nnGraph *g) {
  long int edge_count = 0;
  for (int i = 0; i < g->size; i++) {
    gNode *nodeA = &g->nodes[i];
    for (auto gi : *(nodeA->nset)) {
      edge_count++;
    }
  }
  float edges_per_node = edge_count / ((float)g->size);
  printf("edge_count=%ld edges_per_node=%f", edge_count, edges_per_node);
}

// refine/prune:
// Experimental, remove link a-b if there exists c so that
// d(a,b) > d(a,c) and d(c,b) > d(a,c)

void refine_graph(DataSet *data, nnGraph *g) {
  gNode *nodeA;
  gNode *nodeB;
  gNode *nodeC;
  float dab, dac, dbc;

  g_timer.tuck("start graph refine");
  int total_count = 0;
  int common_n_count = 0;

  vector<pair<int, int>> to_del;

  for (int i = 0; i < g->size; i++) {
    nodeA = &g->nodes[i];
    int a = i;

    // For all nodes a, loop all neighbor nodes b

    for (auto gi : *(nodeA->nset)) {
      // for (int j = 0; j < node->neighbors->size; j++) {
      // gItem *gi = (gItem *)ll_get_item(node->neighbors, j);
      nodeB = &g->nodes[gi->id];
      int b = gi->id;
      dab = gi->dist;

      // For all nodes b, loop all neighbor nodes c
      // (neighbors of neighbors of a)
      // for (int k = j + 1; k < node->neighbors->size; k++) {

      for (auto gi2 : *(nodeA->nset)) {
        int c = gi2->id;
        if (b == c) {
          continue;
        }
        dac = gi2->dist;
        total_count++;
        // printf("a=%d b=%d c=%d dab=%f dac=%f dbc=%f\n", a, b, c, dab, dac, dbc);
        auto it = nodeB->nset->find(gi2);
        if (it != nodeB->nset->end()) {
          dbc = (*it)->dist;
          // if (nng_has_neighbor(g, b, c)) {
          // b and c already neighbors

          // Find largest link disrecarding equal cases
          if (dab > dac) {
            if (dab > dbc) {
              // dab largest
              to_del.emplace_back(a, b);
            } else if (dab < dbc) {
              to_del.emplace_back(b, c);
              // dbc largest
            }
          } else {
            if (dac > dbc) {
              // dac largest
              to_del.emplace_back(a, c);
            } else if (dac < dbc) {
              to_del.emplace_back(b, c);
              // dbc largest
            }
          }

          // printf("b and c already neighbors\n");
          common_n_count++;
          continue;
        }

        // dcb = distance(data, c, b);
        // dac = gi2->dist;

        // if ((dab > dcb || dac > dcb)) {
        // }
      }
    }
    for (vector<pair<int, int>>::const_iterator it = to_del.begin(); it != to_del.end(); it++) {
      // printf("delete %d %d\n", it->first, it->second);
      // nng_remove_neighbor(g, it->first, it->second);
      // nng_remove_neighbor(g, it->second, it->first);
    }
    to_del.clear();
  }

  g_timer.tuck("end graph refine");
  // printf("total_count:%d common_n_count:%d to_delete:%d\n", total_count, common_n_count,
  // to_del.size());
}

void prune_neighbors2(nnGraph *g, int pid) {
  gNode *nodeA;
  gNode *nodeB;
  gNode *nodeC;
  float dab, dac, dbc;

  // g_timer.tuck("start graph refine");
  int total_count = 0;
  int common_n_count = 0;
  int num_pruned = 0;

  vector<pair<int, int>> to_del;

  nodeA = &g->nodes[pid];

  // For all nodes a, loop all neighbor nodes b

  for (auto gi : *(nodeA->nset)) {
    g->nodes[gi->id].visited = g->cur_iter;
    g->nodes[gi->id].visited_dist = gi->dist;
  }
  int a = pid;
  for (auto gi : *(nodeA->nset)) {
    // for (int j = 0; j < node->neighbors->size; j++) {
    // gItem *gi = (gItem *)ll_get_item(node->neighbors, j);
    nodeB = &g->nodes[gi->id];
    int b = gi->id;
    dab = gi->dist;

    // For all nodes b, loop all neighbor nodes c
    // (neighbors of neighbors of a)
    // for (int k = j + 1; k < node->neighbors->size; k++) {

    for (auto gi2 : *(nodeB->nset)) {
      int c = gi2->id;
      if (b == c) {
        continue;
      }
      // c, neighbor of b is also neighbor of a
      if (g->nodes[c].visited == g->cur_iter) {
        dbc = gi2->dist;
        dac = g->nodes[c].visited_dist;

        int rem1, rem2;
        // Find the longest distance in the triangle
        if (dac > dbc) {
          if (dac > dab) {
            rem1 = a;
            rem2 = c;
            g->nodes[c].visited = -1;
          } else if (dac < dab) {
            rem1 = a;
            rem2 = b;
            g->nodes[b].visited = -1;
          }
        } else if (dac < dbc) {
          if (dbc > dab) {
            rem1 = b;
            rem2 = c;
          } else if (dbc < dab) {
            rem1 = a;
            rem2 = b;
            g->nodes[b].visited = -1;
          }
        }

        // if (dab > dbc && dab < dac) {
        // if (dac > dbc && dab < dac) {

        if (g->nodes[rem1].nearest->id != rem2 && g->nodes[rem2].nearest->id != rem1) {
          to_del.emplace_back(rem1, rem2);
          num_pruned++;
          break;
        }
      }
    }
  }

  for (vector<pair<int, int>>::const_iterator it = to_del.begin(); it != to_del.end(); it++) {
    // printf("delete %d %d\n", it->first, it->second);
    nng_remove_neighbor(g, it->first, it->second);
    nng_remove_neighbor(g, it->second, it->first);
  }
  to_del.clear();
  if (g_options.verbose > 1) {
    printf("num_pruned:%d\n", num_pruned);
  }
  // to_del.size());

  // g_timer.tuck("end graph refine");
  // printf("total_count:%d common_n_count:%d to_delete:%d\n", total_count, common_n_count,
  // to_del.size());
}

void prune_neighbors(nnGraph *g, int pid) {
  gNode *nodeA;
  gNode *nodeB;
  gNode *nodeC;
  float dab, dac, dbc;

  // g_timer.tuck("start graph refine");
  int total_count = 0;
  int common_n_count = 0;
  int num_pruned = 0;

  vector<pair<int, int>> to_del;

  nodeA = &g->nodes[pid];

  // For all nodes a, loop all neighbor nodes b

  int a = pid;
  for (auto gi : *(nodeA->nset)) {
    // for (int j = 0; j < node->neighbors->size; j++) {
    // gItem *gi = (gItem *)ll_get_item(node->neighbors, j);
    nodeB = &g->nodes[gi->id];
    int b = gi->id;
    dab = gi->dist;

    if (gi->id == nodeA->nearest_id) {
      continue;
    }

    // For all nodes b, loop all neighbor nodes c
    // (neighbors of neighbors of a)
    // for (int k = j + 1; k < node->neighbors->size; k++) {

    for (auto gi2 : *(nodeA->nset)) {
      int c = gi2->id;
      if (b == c) {
        continue;
      }
      dac = gi2->dist;
      total_count++;
      // printf("a=%d b=%d c=%d dab=%f dac=%f dbc=%f\n", a, b, c, dab, dac, dbc);
      // Check if our neighbor is closer to another neighbor
      auto it = nodeB->nset->find(gi2);
      if (it != nodeB->nset->end()) {
        dbc = (*it)->dist;

        int rem1, rem2;
        if (dac > dbc) {
          if (dac > dab) {
            rem1 = a;
            rem2 = c;
          } else if (dac < dab) {
            rem1 = a;
            rem2 = b;
          }
        } else if (dac < dbc) {
          if (dbc > dab) {
            rem1 = b;
            rem2 = c;
          } else if (dbc < dab) {
            rem1 = a;
            rem2 = b;
          }
        }

        // if (dab > dbc && dab < dac) {
        // if (dac > dbc && dab < dac) {

        if (g->nodes[rem1].nearest->id != rem2 && g->nodes[rem2].nearest->id != rem1) {
          to_del.emplace_back(rem1, rem2);
          num_pruned++;
          break;
        }
        // }

        // common_n_count++;
        continue;
      }
    }
  }

  for (vector<pair<int, int>>::const_iterator it = to_del.begin(); it != to_del.end(); it++) {
    // printf("delete %d %d\n", it->first, it->second);
    nng_remove_neighbor(g, it->first, it->second);
    nng_remove_neighbor(g, it->second, it->first);
  }
  to_del.clear();
  if (g_options.verbose > 1) {
    printf("num_pruned:%d\n", num_pruned);
  }
}

void rpdiv_recurse_queue(linkedList *qu, queueItem *qi) {
  int *left_arr;
  int *right_arr;
  int *left_arr2;
  int *right_arr2;
  int left_size = 0;
  int right_size = 0;

  DataSet *data = qi->data;
  linkedList *ll = qi->ll;
  linkedListNode *llnode = qi->llnode;
  int *input_arr = qi->input_arr;
  int *input_arr2 = qi->input_arr2;
  int input_arr_size = qi->input_arr_size;
  int is_left_child = qi->is_left_child;
  int uncle_id = qi->uncle_id;

  int randind_A, randind_B;
  randind_A = input_arr[(int)(floor(input_arr_size * RAND_FLOAT() - 0.000001))];
  randind_B = randind_A;
  while (randind_B == randind_A) {
    randind_B = input_arr[(int)(floor(input_arr_size * RAND_FLOAT() - 0.000001))];
  }

  // Put B closer to uncle if left child
  // A closer to uncle if right child
  if (uncle_id != -1 && g_options.uncle_adjustment == 1) {
    float A_to_uncle = distance(data, uncle_id, randind_A);
    float B_to_uncle = distance(data, uncle_id, randind_B);
    int tmpi;
    if (is_left_child == 1 && A_to_uncle < B_to_uncle) {
      tmpi = randind_B;
      randind_B = randind_A;
      randind_A = tmpi;
      // printf("switch 1\n");
    }
    if (is_left_child == 0 && A_to_uncle > B_to_uncle) {
      tmpi = randind_B;
      randind_B = randind_A;
      randind_A = tmpi;
      // printf("switch 2\n");
    }
  }

  if (uncle_id != -1 && g_options.uncle_adjustment == 2) {
    int tmpi;
    if (is_left_child == 1) {
      uncle_id = *((int *)llnode->next->content);
      float A_to_uncle = distance(data, uncle_id, randind_A);
      float B_to_uncle = distance(data, uncle_id, randind_B);
      if (A_to_uncle < B_to_uncle) {
        tmpi = randind_B;
        randind_B = randind_A;
        randind_A = tmpi;
        // printf("switch 1\n");
      }
    }
    if (is_left_child == 0) {
      uncle_id = *((int *)llnode->prev->content);
      float A_to_uncle = distance(data, uncle_id, randind_A);
      float B_to_uncle = distance(data, uncle_id, randind_B);
      if (A_to_uncle > B_to_uncle) {
        tmpi = randind_B;
        randind_B = randind_A;
        randind_A = tmpi;
        // printf("switch 1\n");
      }
    }
  }

  if (uncle_id != -1 && g_options.uncle_adjustment == 3) {
    int tmpi;
    if (llnode->next != NULL && llnode->prev != NULL) {

      int previd = *((int *)llnode->prev->content); // previous in the chain
      int nextid = *((int *)llnode->next->content); // next in the chain
      float A_to_prev = distance(data, previd, randind_A);
      float A_to_next = distance(data, nextid, randind_A);
      float B_to_prev = distance(data, previd, randind_B);
      float B_to_next = distance(data, nextid, randind_B);

      if (A_to_prev + B_to_next > A_to_next + B_to_prev) {
        tmpi = randind_B;
        randind_B = randind_A;
        randind_A = tmpi;
      }
    }
  }

  int class_A_count = 0;
  int class_B_count = 0;

  for (int i = 0; i < input_arr_size; i++) {
    float dist_A = distance(data, input_arr[i], randind_A);
    float dist_B = distance(data, input_arr[i], randind_B);

    if (dist_A < dist_B) {
      // put to left side
      input_arr2[class_A_count] = input_arr[i];
      class_A_count++;
    } else {
      // put to right side
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

  // if (left_size == 0 || right_size==0) {
  // printf("left_size=%d right_size=%d\n", left_size, right_size);
  // for (int i = 0; i < input_arr_size; i++) {
  // float dist_A = distance(data, input_arr[i], randind_A);
  // float dist_B = distance(data, input_arr[i], randind_B);
  // printf("input_arr[i]=%d dA=%f dB=%f", input_arr[i], dist_A, dist_B);
  // }
  // }

  *((int *)llnode->content) = randind_A;
  int *tmpcontent = (int *)malloc(sizeof(int));
  *tmpcontent = randind_B;
  ll_add_after_node(ll, llnode, tmpcontent);

  if (left_size > 0) {
    if (left_size >= 2) {
      queueItem *qileft = (queueItem *)malloc(sizeof(queueItem));
      qileft->data = data;
      qileft->ll = ll;
      qileft->llnode = llnode;
      qileft->input_arr = left_arr2;
      qileft->input_arr2 = left_arr;
      qileft->input_arr_size = left_size;
      qileft->is_left_child = 1;
      qileft->uncle_id = randind_B;
      ll_add_node(qu, qileft);
    }
    if (right_size >= 2) {
      queueItem *qiright = (queueItem *)malloc(sizeof(queueItem));
      qiright->data = data;
      qiright->ll = ll;
      qiright->llnode = llnode->next;
      qiright->input_arr = right_arr2;
      qiright->input_arr2 = right_arr;
      qiright->input_arr_size = right_size;
      qiright->is_left_child = 0;
      qiright->uncle_id = randind_A;
      ll_add_node(qu, qiright);
    }
  }
}

// Random point division for TSP solution
// recursion implemented using a queue
void rpdiv_queue(DataSet *data, int *ind_arr, int *ind_arr2) {
  linkedList *qu; // queue
  qu = initLinkedList();

  linkedList *ll = initLinkedList();
  int *tmp = (int *)malloc(sizeof(int));
  *tmp = -1;
  linkedListNode *firstnode = ll_add_node(ll, tmp);

  queueItem *qi;
  queueItem *qiroot = (queueItem *)malloc(sizeof(queueItem));
  qiroot->data = data;
  qiroot->ll = ll;
  qiroot->llnode = NULL;
  qiroot->input_arr = ind_arr;
  qiroot->input_arr2 = ind_arr2;
  qiroot->input_arr_size = data->size;
  qiroot->is_left_child = 1;
  qiroot->uncle_id = -1;
  linkedListNode *top;

  qiroot->llnode = ll->root;

  ll_add_node(qu, qiroot);
  int i = 1;
  while (qu->size > 0) {
    // printf("i=%d qu->size=%d\n",i,qu->size);
    top = ll_pop_first_node(qu);
    qi = (queueItem *)top->content;
    rpdiv_recurse_queue(qu, qi);
    free(top);
    free(qi);
    i++;
  }
  ll_free_list(ll);
}

// int __attribute__((noinline)) grand() { return rand(); }
int grand() { return rand(); }
float calc_clu_dist(nnGraph *g, int p1, int p2) {

  int numsample = g_options.num_samples;
  // int numsample = 50;
  int size1 = g->nodes[p1].stash->size();
  int size2 = g->nodes[p2].stash->size();
  float d;
  float d2;
  float _dist = 0.0;

  // g_stat.num_calc_clu_dist++; //TODO: enable

  if (g->data->type == T_NUMERICAL && g_options.mean_calculation) {
    d2 = L2dist(g->nodes[p1].mean, g->nodes[p2].mean, g->data->dimensionality);
    // printf("d = %f, ", d);
    // printf("m = %f %f %f %f ",
    // g->nodes[p1].mean[0],g->nodes[p2].mean[0],g->nodes[p1].mean[g->data->dimensionality-1],g->nodes[p2].mean[g->data->dimensionality-1]
    // ); printf("\n");
    float Sa = g->nodes[p1].internalSum;
    float Sb = g->nodes[p2].internalSum;
    float Na = size1;
    float Nb = size2;
    d2 = scale_dist(d2);
    // d2 = d2 * (((float)size1) * ((float)size2));
    // Convert from distances between centroids (d2) to sum of squared distances between clusters
    d2 = d2 * Na * Nb + (Sa * Nb) / Na + (Na * Sb) / Nb;
    return d2;
    // dist*Na*Nb + (Sa*Nb)/Na + (Na*Sb)/Nb
    // printf("d2 = %f, ", d);
    // TODO: in case T_NUMERICAL, but non-euclidean distance
    // return d;
  }

  if (numsample > 0 && size1 * size2 > numsample) {
    for (int i = 0; i < numsample; i++) {
      int rnd1 = grand() % size1;
      int rnd2 = grand() % size2;
      int idA = (*(g->nodes[p1].stash))[rnd1];
      int idB = (*(g->nodes[p2].stash))[rnd2];
      // int idB = g->nodes[p2].stash[rnd2];
      float dtmp = distance(g->data, idA, idB);
      // _dist += dtmp * dtmp;
      _dist = dist_combine(_dist, scale_dist(dtmp));
    }
    // d = (_dist / numsample) * (size1 + size2);
    if (g_options.costf == 1 || g_options.costf == 3 || g_options.costf > 4) {
      d = (_dist / numsample) * (((float)size1) * ((float)size2));
    } else {
      d = _dist;
    }
  } else {
    // printf("samples=%d\n", g_options.num_samples);
    for (int idA : *(g->nodes[p1].stash)) {
      for (int idB : *(g->nodes[p2].stash)) {
        float dtmp = distance(g->data, idA, idB);
        // _dist += dtmp * dtmp;
        _dist = dist_combine(_dist, scale_dist(dtmp));
        // printf("_dist[%d,%d] = %f ",idA,idB,_dist);
      }
    }

    d = _dist;
  }

  // float p = 0.8;
  // if(d/d2 < 1-p || d/d2 > 1+p) {
  // printf("d=%f d2=%f\n",d,d2);

  return d;
}

void nng_update_nearest(nnGraph *g, int p1) {
  gNode *p1node = &(g->nodes[p1]);
  p1node->nearest_id = 0;
  p1node->nearest_dist = FLT_MAX;
  for (auto gi : *(p1node->nset)) {
    // printf("p1=%d gi=%d dist=%f\n", p1, gi->id, gi->dist);
    if (gi->cost < p1node->nearest_dist) {
      // p1node->nearest_dist = gi->dist;
      p1node->nearest_dist = gi->cost;
      p1node->nearest_id = gi->id; // TODO: remove and use only ->nearest
      p1node->nearest = gi;
    }
  }
}

int nng_check_nearest(nnGraph *g, int p1) {
  gNode *p1node = &(g->nodes[p1]);
  // p1node->nearest_id = 0;
  // p1node->nearest_dist = FLT_MAX;
  int changed = 0;
  for (auto gi : *(p1node->nset)) {
    // printf("p1=%d gi=%d dist=%f p1node->nearest_id=%d\n", p1, gi->id, gi->dist,
    // p1node->nearest_id);
    if (gi->cost < p1node->nearest_dist) {
      // p1node->nearest_dist = gi->dist;
      p1node->nearest_dist = gi->cost;
      p1node->nearest_id = gi->id; // TODO: remove and use only ->nearest
      p1node->nearest = gi;
      changed = 1;
    }
  }
  return changed;
}

void printNodeData(nnGraph *g, int id) {

  printf("==== Node info id=%d ==== \n", id);
  printf("Nearest: %d (%f)\n", g->nodes[id].nearest_id, g->nodes[id].nearest_dist);
  int i = 0;
  for (gItem *gi : *(g->nodes[id].nset)) {
    i++;
    printf("%d: %d (%f)\n", i, gi->id, gi->dist);
  }
  printf("=========================\n");
  return;
}

// Perform clustering using the TSP-graph or any connected graph
int *cluster_tspg(DataSet *data, nnGraph *g, int k, vector<vector<float>> *centroids) {
  printf("Clustering using tspg graph\n");
  g->data = data;
  gNode *node;
  int rvar = 0;
  int *partition = (int *)malloc(sizeof(int) * g->size);

  nodeHeap *H = new nodeHeap();

  for (int i = 0; i < g->size; i++) {
    node = &(g->nodes[i]);
    g->nodes[i].heap_index = -1;
    // node->nearest_dist = 0;
    for (auto gi : *(g->nodes[i].nset)) {
      calc_cost(g, gi);
    }
    nng_update_nearest(g, node->id);
    H->insert((void *)node, &(node->heap_index));
    // printf("Initial sanity check:\n");
    // g->nodes[i].nearesth->checkSanity();
  }

  int num_clu = g->size;
  int i = 0;
  for (i = 0; num_clu > k; i++) {
    g->cur_iter = i;

    node = (gNode *)H->data[1];
    if (node->outdated) {
      if (g_options.verbose > 2) {
        printf("i=%d num_clu=%d MERGE p1=%d p2=%d nset_size=%ld stash_size=%ld (Dirty)\n", i,
               num_clu, node->id, node->nearest_id, node->nset->size(), node->stash->size());
      }
      nng_update_nearest(g, node->id);
      H->update(1);
      node->outdated = 0;
      continue;
    }

    debug_assert(nng_check_nearest(g, node->id) == 0);

    if (g_options.verbose > 1) {
      printf("i=%d num_clu=%d MERGE p1=%d p2=%d nset_size=%ld stash_size=%ld", i, num_clu, node->id,
             node->nearest_id, node->nset->size(), node->stash->size());

      float nsum = 0.0;
      for (int i = 0; i < num_clu; i++) {
        gNode *nodetmp = (gNode *)H->data[i + 1];
        nsum += nodetmp->nset->size();
      }
      printf(" mean_neighb=%f", nsum / num_clu);
      printf("\n");
    }

    // if (node->nearest_id == 4906 || node->id == 4906) {
    // printNodeData(g, node->id);
    // printNodeData(g, node->nearest_id);
    // }

    assert(node->outdated != 1);
    assert(node->id != node->nearest->id);
    assert(node->id < g->size && node->id >= 0);
    assert(node->nearest->id < g->size && node->nearest->id >= 0);
    assert(node->nearest_id == node->nearest->id);

    nng_merge_nodes(g, H, node->id, node->nearest_id);

    num_clu--;

    if (g_options.time_limit > 0 && g_timer.get_time() > g_options.time_limit) {
      printf("Exit due to time limit\n");
      exit(1);
    }
  }

  // printf("i=%d\n", i);
  // printf("heap size = %d \n", H->size);

  int cluid = 1;
  while (H->getSize() > 0) {
    node = (gNode *)H->data[1];
    H->remove(1);

    for (int idA : *(node->stash)) {
      partition[idA] = cluid;
    }
    if (centroids != NULL && g_options.mean_calculation) {
      for (int i_dim = 0; i_dim < data->dimensionality; i_dim++) {
        // printf("%f ", node->mean[i_dim]);
        (*centroids)[cluid - 1][i_dim] = node->mean[i_dim];
      }
    }
    cluid++;
  }

  printf("FINAL=1 time=%f ", g_timer.get_time());
  graph_stat(g);
  printf("\n");

  return partition;
}

nnGraph *create_complete_graph(DataSet *data, nnGraph *g) {
  g_timer.tuck("Creating complete graph");
  float total_dist = 0;
  for (int a = 0; a < data->size - 1; a++) {
    for (int b = a + 1; b < data->size; b++) {
      float _dist = scale_dist(distance(data, a, b));
      total_dist = _dist;
      nng_add_mutual_neighbor2(g, a, b, _dist);
    }
  }

  g_timer.tuck("Graph created");
  return g;
}

nnGraph *create_tspg(DataSet *data, nnGraph *g, int num_tsp, linkedList **ll_ret) {
  kNNGraph *knng = NULL;
  int update_count = 0;
  float update_portion = 0.0;

  int k_increment = 2;
  int run_nndes = 0;
  float update_portion_nndes = 0;
  int i_iter;

  // Two copies of tree. Optimization to avoid memory alloc/dealloc in future steps
  int *ind_arr = (int *)safemalloc(sizeof(int) * data->size);
  int *ind_arr2 = (int *)safemalloc(sizeof(int) * data->size);

  linkedList *ll = initLinkedList();

  if (g == NULL) {
    g = init_nnGraph(data->size);
  }

  g->data = data;
  // Calculate initial mean vectors if possible

  if (g->data->type == T_NUMERICAL && g_options.mean_calculation) {
    for (int i_data = 0; i_data < data->size; i_data++) {
      g->nodes[i_data].mean = (float *)malloc(sizeof(float) * data->dimensionality);
      for (int i_dim = 0; i_dim < data->dimensionality; i_dim++) {
        g->nodes[i_data].mean[i_dim] = data->data[i_data][i_dim];
      }
    }
  }

  float total_dist_sum = 0.0;
  for (i_iter = 0; i_iter < num_tsp; i_iter++) {
    printf("iter=%d ", i_iter);
    g_timer.tuck("time");
    update_count = 0;
    int update_count_nndes = 0;

    for (int i_data = 0; i_data < data->size; i_data++) {
      ind_arr[i_data] = i_data;
    }

    ll = initLinkedList();
    int *tmp = (int *)malloc(sizeof(int));
    *tmp = -1;
    linkedListNode *firstnode = ll_add_node(ll, tmp);
    rpdiv_queue(data, ind_arr, ind_arr2);

#ifdef DISABLED00
    int *map = (int *)malloc(sizeof(int) * data->size);
    for (int i_data = 0; i_data < data->size; i_data++) {
      map[i_data] = ind_arr[i_data];
    }
    ll_add_node(ll, (void *)map);
#endif

    float total_dist = 0;
    for (int i_data = 0; i_data < data->size - 1; i_data++) {
      int a, b;
      a = ind_arr[i_data];
      b = ind_arr[i_data + 1];
      float d_tmp = distance(data, a, b);
      // printf("a=%d b=%d d=%f\n",a,b,d_tmp);
      float _dist = scale_dist(d_tmp);
      total_dist = _dist;
      nng_add_mutual_neighbor2(g, a, b, _dist);
    }
    total_dist_sum += total_dist;
    printf("I=%d total_dist=%f\n", i_iter, total_dist);
  }
  printf("\nmean_tsp_length=%f\n", total_dist_sum / (i_iter + 1));
  // graph_stat(g);
  if (g_options.refine_iter > 0) {
    refine_graph(data, g);
    // graph_stat(g);
  }

  gNode *node;

  free(ind_arr);
  free(ind_arr2);

  /*ll_free_list(ll);*/
  (*ll_ret) = ll;
  return g;
}

#define MIN(a, b) (((a) < (b)) ? (a) : (b))
#define MAX(a, b) (((a) > (b)) ? (a) : (b))

float scale_dist(float dtmp) {

  switch (g_options.scale_method) {
  case 2:
    return dtmp * dtmp;
  case 1:
    return dtmp;
  default:
    return pow(dtmp, g_options.scale_method);
  }
}

float dist_combine(float a, float b) {
  switch (g_options.costf) {
  case 2:
    return MAX(a, b);
    break;

  case 4:
    return MIN(a, b);
    break;

  default:
    return a + b;
  }
}
void calc_cost(nnGraph *g, gItem *gi) {
  int p1 = gi->pair->id;
  int p2 = gi->id;
  float p1size = g->nodes[p1].stash->size();
  float p2size = g->nodes[p2].stash->size();
  float sa, sb;

  float dtmp;

  switch (g_options.costf) {
  case 3:
    dtmp = p1size + p2size;
    dtmp = dtmp * dtmp;
    sa = g->nodes[p1].internalSum;
    sb = g->nodes[p2].internalSum;

    // gi->cost = (sa+sb+gi->dist) / dtmp - sa/(p1size*p1size) - sb/(p2size*p2size) ;
    gi->cost = gi->dist / (p1size * p2size);
    break;

  case 5: // TSE
    // (Sa+Sb+Sab)/(Nb+Na) - Sa/Na - Sb/Nb
    sa = g->nodes[p1].internalSum;
    sb = g->nodes[p2].internalSum;
    gi->cost = (sa + sb + gi->dist) / (p1size + p2size) - sa / p1size - sb / p2size;
    if (gi->cost < 0 && g_options.verbose > 1) {
      if (g_options.verbose > 1) {
        printf("gicost=%f (gi->dist=%f p1=%d p2=%d, sa=%f sb=%f p1size=%f p2size=%f)\n", gi->cost,
               gi->dist, p1, p2, sa, sb, p1size, p2size);
      }
    }
    break;

  default:
    gi->cost = gi->dist;
  }

  // Update merge cost also for the other node
  gi->pair->cost = gi->cost;

  return;
}

void nng_merge_nodes(nnGraph *g, nodeHeap *H, int p1, int p2) {

  gNode *p1node = &(g->nodes[p1]);
  gNode *p2node = &(g->nodes[p2]);
  int p1size = g->nodes[p1].stash->size();
  int p2size = g->nodes[p2].stash->size();
  int newsize = p1size + p2size;
  float newdist = 0;
  // int use_heap = 1;

  debug_assert(H->isHeap());

  if (g_options.prune_strategy == 2) {
    if (g->nodes[p1].nset->size() > 20) {
      prune_neighbors(g, p1);
    }
    if (g->nodes[p2].nset->size() > 20) {
      prune_neighbors(g, p2);
    }
  }

  // assert(p2node->outdated != 1);
  debug_assert(p1node->outdated != 1);

  for (auto gi : *(g->nodes[p1].nset)) {
    gi->visited = p2; // TODO: check if =0 gives same
  }
  // printf("MERGE p1=%d p2=%d\n",p1,p2);

  debug_assert(H->isHeap());
  // Loop neighbors of p2
  // Merge neighbor sets of p1 and p2
  int i3 = 0;
  for (auto gi : *(g->nodes[p2].nset)) {
    i3++;

    // Remove p2 from neighbors of p2
    auto posn = g->nodes[gi->id].nset->find(gi->pair);
    g->nodes[gi->id].nset->erase(posn);

    // g->nodes[gi->id].nset->erase(gi->iterO); //TODO??

    if (gi->id == p1) {
      continue;
    }

    int sizeC = g->nodes[gi->id].stash->size();

    auto it = g->nodes[p1].nset->find(gi);
    if (it == g->nodes[p1].nset->end()) {
      // The case where neighbor of p2 is not a neighbor of p1

      float p1dist;
      p1dist = calc_clu_dist(g, p1, gi->id);
      newdist = dist_combine(gi->dist, p1dist);

      gItem *gi_p1 = nng_add_mutual_neighbor2(g, p1, gi->id, newdist /*dist*/);

    } else {

      // The case where gi is neighbor for both merged nodes
      (*it)->visited = -1;

      newdist = dist_combine((*it)->dist, gi->dist);
      ((gItem *)(*it)->pair)->dist = (*it)->dist = newdist;
    }
  }

  debug_assert(H->isHeap());
  // p1node->nearest_id = 0;
  // p1node->nearest_dist = FLT_MAX;
  // Loop all neighbors of new merged node.
  for (auto gi : *(g->nodes[p1].nset)) {

    float newdist = 0;
    // Neighbor of p1, but not neighbor of p2
    if (gi->visited == p2) {
      float p2dist;

      p2dist = calc_clu_dist(g, p2, gi->id);
      // newdist = gi->dist + p2dist;
      newdist = dist_combine(gi->dist, p2dist);

      gi->dist = newdist;
      ((gItem *)gi->pair)->dist = gi->dist;
      // calc_cost(g, gi); //TODO:enable
    }

    if (g->nodes[gi->id].nearest_id == p1 || g->nodes[gi->id].nearest_id == p2) {
      g->nodes[gi->id].outdated = 1;
    }
  }
  // H->update(p1node->heap_index);

  debug_assert(H->isHeap());
  // Merge stash of p1 and p2
  g->nodes[p1].stash->insert(g->nodes[p1].stash->end(), g->nodes[p2].stash->begin(),
                             g->nodes[p2].stash->end());

  float w = ((float)p1size) / (p1size + p2size);

  if (g->data->type == T_NUMERICAL && g_options.mean_calculation) {
    for (int i_dim = 0; i_dim < g->data->dimensionality; i_dim++) {
      p1node->mean[i_dim] = p1node->mean[i_dim] * w + p2node->mean[i_dim] * (1 - w);
    }
  }

  p1node->internalSum += p2node->internalSum + p1node->nearest->dist;

  // TODO: integrate with code abowe
  // Loop all neighbors of new merged node.

  debug_assert(H->isHeap());
  for (auto gi : *(g->nodes[p1].nset)) {
    calc_cost(g, gi);
  }

  debug_assert(H->isHeap());
  nng_update_nearest(g, p1);

  g->nodes[p2].stash->clear();
  g->nodes[p2].nset->clear();
  p2node->outdated = 1;

  H->update(g->nodes[p1].heap_index);
  H->remove(g->nodes[p2].heap_index);

  debug_assert(H->isHeap());

  if (g_options.prune_strategy == 1) {
    // if (g->nodes[p1].nset->size() > 20) {
    // prune_neighbors(g, p1);
    prune_neighbors2(g, p1);
    // }
  }
}
