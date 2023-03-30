
namespace tspg {

float L2df::operator()(int i, int j) const {

  // float L2dist(float *p1_idx, float *p2_idx, int D) {

  float *p1_idx = data->data[i];
  float *p2_idx = data->data[j];
  // (float *)*(DS->data + idx)
  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < data->dimensionality; i++) {
    tmp = *p1_idx - *p2_idx;
    dist += tmp * tmp;
    p1_idx++;
    p2_idx++;
  }
  return sqrt(dist);
}

float L1df::operator()(int i, int j) const {
  float *p1_idx = data->data[i];
  float *p2_idx = data->data[j];
  float tmp = 0;
  float dist = 0;
  for (int i = 0; i < data->dimensionality; i++) {
    tmp = *p1_idx - *p2_idx;
    dist += abs(tmp);
    p1_idx++;
    p2_idx++;
  }
  return dist;
}


GraphDistance::~GraphDistance() {
  for (int i = 0; i < ncodes; ++i) {
    delete[] dm[i];
  }
  delete[] dm;
}

// GraphDistance::GraphDistance(nnGraph *graph_) : graph(graph_) { N = graph->size; }
GraphDistance::GraphDistance(nnGraph *graph_, DataSet *data_, int _ncodes)
    : graph(graph_), data(data_), ncodes(_ncodes) {
  size = data->size;

  dm = new float *[ncodes];
  for (int i = 0; i < ncodes; ++i) {
    dm[i] = new float[ncodes];
  }

  for (int i = 0; i < ncodes; ++i) {
    for (int j = 0; j < ncodes; ++j) {
      gItem *item;
      float v = 1.0;
      if (i < graph->size && j < graph->size) {
        item = nng_get_neighbor(graph, i, j);
        if (item != NULL) {
          v = item->dist;
          // printf("dist:%f\n",item->dist);
        }
      }
      float scaledv = (1 + v) / v;
      dm[i][j] = scaledv;
    }
  }

  // item = nng_get_neighbor(graph, code1, code2);
}

float GraphDistance::operator()(int a, int b) const {
  // sd->bigrams[a]
  // sd->setSize[a]
  gItem *item;

  float ssum = 0.0;
  float t = 1.0;
  int ncodes1 = data->setSize[a];
  int ncodes2 = data->setSize[b];
  for (int i = 0; i < ncodes1; i++) {
    for (int j = 0; j < ncodes2; j++) {
      float v = 1.0;
      int code1 = data->bigrams[a][i];
      int code2 = data->bigrams[b][j];
      // graph->nodes[i];
      if (code1 == code2) {
        v = 1.0;
      } else {
        // v = 1.0;
        // if (item != NULL) {
        // v = item->dist;
        // // printf("dist:%f\n",item->dist);
        // }
        // v = (1 + v) / v; // TODO: Scaling as parameter
        v = dm[i][j];
      }
      ssum += v;
    }
  }

  t = ssum / (ncodes1 * ncodes2);
  return t;
}

}
