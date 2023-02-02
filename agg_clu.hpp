typedef struct queueItem {
  linkedListNode *llnode;
  linkedList *ll;
  int *input_arr;
  int *input_arr2;
  int input_arr_size;
  DataSet *data;
  int level;
  kNNGraph *knng;
  int is_left_child;

  int *update_count;
  int uncle_id;

} queueItem;

class nodeHeap : public Heap {
public:
  int compare(void *_a, void *_b) {
    // printf("comp\n");

    gNode *a = (gNode *)_a;
    gNode *b = (gNode *)_b;

    if (a->nearest_dist > b->nearest_dist) {
      return 1;
    } else if (a->nearest_dist < b->nearest_dist) {
      return -1;
    } else {
      // printf("EQUALS\n");
      return 0;
    }
  }
};

int CmpDistP(void *a, void *b, void *info);
void merge_graphs_knn(nnGraph *g, kNNGraph *kNN);
void graph_stat(nnGraph *g);
void refine_graph(DataSet *data, nnGraph *g);
void prune_neighbors2(nnGraph *g, int pid);
void prune_neighbors(nnGraph *g, int pid);
void rpdiv_recurse_queue(linkedList *qu, queueItem *qi);
void rpdiv_queue(DataSet *data, int *ind_arr, int *ind_arr2);
int grand(); 
float calc_clu_dist(nnGraph *g, int p1, int p2);
int nng_check_nearest(nnGraph *g, int p1);
void printNodeData(nnGraph *g, int id);
float scale_dist(float dtmp);
float dist_combine(float a, float b);
void calc_cost(nnGraph *g, gItem *gi);
void nng_merge_nodes(nnGraph *g, nodeHeap *H, int p1, int p2);

