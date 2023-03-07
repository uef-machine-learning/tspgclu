
#ifndef DATASET_H
#define DATASET_H

#ifdef _PYTHON_LIB
#include <Python.h>
#endif

#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include <sstream>
#include <string>
#include <cstring>
#include <vector>
#include <iostream>

#include <map>
#include <algorithm>

int *calc_ngrams(const char *str1, int ngram);

extern struct knng_options g_options;

using namespace std;

// TODO:
#define MEM_ALIGNMENT 32

struct DataSet {
  int size;
  int dimensionality;
  int vector_size;
  float elem_min;
  float elem_max;
  float **data;

  vector<string> *strings;
  int **bigrams;
  int **trigrams;
  int type;
  int distance_type;
  int *setSize; // For set data
#ifdef _PYTHON_LIB
  PyObject **pyids;
  PyObject *pyobj;
  PyObject *pydf; // Distance function defined in python
#endif
};

template <typename M> class OracleL2 {
  const M &m;

public:
  OracleL2(const M &m_) : m(m_) {}
  float operator()(int i, int j) const __attribute__((noinline));
};

template <typename M> float OracleL2<M>::operator()(int i, int j) const {
  const typename M::value_type *first1 = m[i];
  const typename M::value_type *first2 = m[j];
  float r = 0.0;
  for (int i = 0; i < m.getDim(); ++i) {
    float v = first1[i] - first2[i];
    r += v * v;
  }
  return sqrt(r);
}

class OracleA {
  // const M &m;
public:
  const int N;
  OracleA(const int N_) : N(N_) {}
  // float operator()(int i, int j) const __attribute__((noinline));
  float operator()(int i, int j) const;
};

float OracleA::operator()(int i, int j) const {
  return i * j;
  // const typename M::value_type *first1 = m[i];
  // const typename M::value_type *first2 = m[j];
  // float r = 0.0;
  // for (int i = 0; i < m.getDim(); ++i) {
  // float v = first1[i] - first2[i];
  // r += v * v;
  // }
  // return sqrt(r);
}

struct nnGraph;
class L2df {
  // const M &m;
public:
  int size;
  nnGraph *graph;
  DataSet *data;
  // GraphDistance(nnGraph *graph_) : graph(graph_) { N = graph->size; }
  // GraphDistance(DataSet *data_);
  L2df(DataSet *data_) : data(data_) { size = data->size; };
  // float operator()(int i, int j) const __attribute__((noinline));
  float operator()(int i, int j) const;
};

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

struct nnGraph;
class GraphDistance {
  // const M &m;
public:
  int size;
  float **dm;
  nnGraph *graph;
  DataSet *data;
  // GraphDistance(nnGraph *graph_) : graph(graph_) { N = graph->size; }
  GraphDistance(nnGraph *graph_, DataSet *data_, int ncodes);
  // float operator()(int i, int j) const __attribute__((noinline));
  float operator()(int i, int j) const;
};

// float GraphDistance::operator()(int a, int b) const {
  // // sd->bigrams[a]
  // // sd->setSize[a]

  // for (int i = 0; i < data->setSize[a]; i++) {
    // for (int j = 0; j < data->setSize[b]; j++) {
    // float v = 1.0;
    // int code1 = data->bigrams[a][i];
    // int code2 = data->bigrams[b][j];
    // // graph->nodes[i];
    // nng_get_neighbor(graph,1,2);
    // }
  // }
  // return a * b - 3.2;
  // // const typename M::value_type *first1 = m[i];
  // // const typename M::value_type *first2 = m[j];
  // // float r = 0.0;
  // // for (int i = 0; i < m.getDim(); ++i) {
  // // float v = first1[i] - first2[i];
  // // r += v * v;
  // // }
  // // return sqrt(r);
// }

// NNDescent<OracleL2<FloatDataset> > nndes(data,N, K, S, oracle, GRAPH_BOTH);

float dice_set_distance(DataSet *sd, int a, int b);
std::istream &safeGetline(std::istream &is, std::string &t);
DataSet *loadSetData(std::string infname);
DataSet *init_DataSet(int size, int dimensionality);
void free_DataSet(DataSet *DS);
float *get_vector(DataSet *DS, int idx);
void set_val(DataSet *DS, int data_i, int elem_i, float val);
float get_val(DataSet *DS, int data_i, int elem_i);
int dump_vector(DataSet *DS, int idx);
float minkowskiDist(float *p1_idx, float *p2_idx, int D, float p);
float L05dist(float *p1_idx, float *p2_idx, int D);
float L1dist(float *p1_idx, float *p2_idx, int D);
float L2dist(float *p1_idx, float *p2_idx, int D);
float cosine_dist(float *p1_idx, float *p2_idx, int D);
float distance(DataSet *P, int p1, int p2);
DataSet *read_lshkit(const char *fname);
DataSet *read_ascii_dataset(const char *fname);
int cmp(const void *a, const void *b);
DataSet *loadStringData(std::string infname);
int *calc_ngrams(const char *str1, int ngram);
double dice_ngram(int *bg1, int *bg2, int setsize1, int setsize2);
float dice_distance_precalc(DataSet *sd, int a, int b);
double dice(const char *str1, const char *str2, int ngram);
float dice_distance(const std::string &s1, const std::string &s2);
unsigned int edit_distance(const std::string &s1, const std::string &s2);
void debugStringDataset(DataSet *DS);

#endif
