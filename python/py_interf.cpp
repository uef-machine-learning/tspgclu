

#include <Python.h>
#include <math.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

#include <stdio.h>
#include <pthread.h>
#include "contrib/argtable3.h"
#include <sys/mman.h> // mmap, munmap

#include <execinfo.h>
#include <signal.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

struct stat2 {
  int num_calc_clu_dist;
  int num_pruned;
};

int g_use_heap = 0;
struct stat2 g_stat;

void print_stat() {
  printf("STAT num_calc_clu_dist=%d num_pruned=%d\n", g_stat.num_calc_clu_dist, g_stat.num_pruned);
}


#include "constants.h"
#include "options.h"

#include "heap.cpp"

#include "timer.hpp"
#include "util.hpp"
#include "globals.h"
#include "dataset.hpp"
#include "linked_list.hpp"
#include "nngraph.hpp"
#include "knngraph.hpp"
#include "agg_clu.hpp"

#include "util.cpp"
#include "dataset.cpp"
#include "knngraph.cpp"
#include "nngraph.cpp"
#include "linked_list.cpp"
#include "options.cpp"
#include "agg_clu.cpp"

DataSet *g_data = NULL;

float *g_distance(int a, int b) { distance(g_data, a, b); }

#include <Python.h>
#include <cstring>
#include <cmath>
#include <fstream>
#include <csignal> // Raise
#include <fstream>
#include <vector>
#include <float.h>
#include <math.h>
#include <numpy/arrayobject.h>
// #ifdef Py_PYTHON_H
// #include "rknng_lib.h"
#include <stdio.h>
// #include "rknng/rknng_lib.h"
// #include "dencl/dencl.hpp"

using namespace std;

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

// extern "C" {

PyObject *array_to_py(int *arr, int N) {
  // Convert c array to python format
  // printf("array_to_py size=%d\n", N);
  PyObject *pyarr = PyList_New(N);
  for (int i = 0; i < N; i++) {
    PyList_SetItem(pyarr, i, Py_BuildValue("i", arr[i]));
  }
  return pyarr;
}

PyObject *py_TSPgClu(PyArrayObject *py_v, int num_clusters, int num_tsp, int dfunc) {

  PyObject *ret;
  PyObject *py_labels;
  PyObject *py_peaks;
  printf("py_TSPgClu\n");

  double *delta;
  int *nearestHighDens;
  double *density;
  // Array *neighborhood_peaks;
  int *peaks;
  int *labels;

  int N = py_v->dimensions[0];
  int D = py_v->dimensions[1];
  // DataSet *DS = init_DataSet(N, D);
  DataSet *data = init_DataSet(N, D);
  // DS->distance_type = dtype;

  printf("V %f %f\n", v(0, 0), v(0, 1));
  for (int i = 0; i < N; i++) {
    for (int j = 0; j < D; j++) {
      set_val(data, i, j, v(i, j));
      if (i < 10) {
        printf(" %f", v(i, j));
      }
    }
    if (i < 10) {
      printf("\n");
    }
  }

  g_options.verbose = 2;
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
  g_options.mean_calculation = 0;

  printf("Algorithm: TSPg-clu\n");

  linkedList *ll;
  nnGraph *nng = init_nnGraph(data->size);

  vector<vector<float>> *centroids;
  nng = create_tspg(data, nng, num_tsp, &ll);

  centroids = new vector<vector<float>>(num_clusters, vector<float>(data->dimensionality, 0));
  int *part = cluster_tspg(data, nng, num_clusters, centroids);

  py_labels = array_to_py(part, N);
  // int arr2[] = {1, 2, 3, 4};
  // ret = PyList_New(2);
  // PyList_SetItem(ret, 0, py_labels);
  // return ret;
  
  return py_labels;
}

PyObject *py_TSPgCluGeneric(PyObject *py_v, int num_clusters, int num_tsp) {

  PyObject *ret;
  PyObject *py_labels;
  // Start counting time
  g_timer.tick();


  int *labels;

  PyObject *pysize = PyObject_GetAttrString(py_v, "size");
  DataSet *data = (DataSet *)safemalloc(sizeof(DataSet));
  data->size = PyLong_AsLong(pysize);
  data->dimensionality = 0;
  data->type = T_CUSTOMDF;
  data->pydf = PyObject_GetAttrString(py_v, "distance");
  int N = data->size;

  data->pyids = (PyObject **)malloc(sizeof(PyObject) * N);
  for (int i = 0; i < N; i++) {
    data->pyids[i] = PyLong_FromLong(i);
  }

  g_options.verbose = 2;
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
  g_options.mean_calculation = 0;

  printf("Algorithm: TSPg-clu\n");

  linkedList *ll;
  nnGraph *nng = init_nnGraph(data->size);

  nng = create_tspg(data, nng, num_tsp, &ll);

  int *part = cluster_tspg(data, nng, num_clusters, NULL);

  // int arr2[] = {1, 2, 3, 4};
  // py_labels = array_to_py((int *)arr2, 3);
  // ret = PyList_New(2);
  // PyList_SetItem(ret, 0, py_labels);
  
  py_labels = array_to_py(part, N);

  return py_labels;
}

extern "C" {

static PyObject *tspg_py(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject *tspg_generic_py(PyObject *self, PyObject *args, PyObject *kwargs);

// Define python accessible methods
static PyMethodDef TSPgCluMethods[] = {
    {"tspg", tspg_py, METH_VARARGS | METH_KEYWORDS, "Cluster using TSP-graph & Ward's method."},
    {"tspg_generic", tspg_generic_py, METH_VARARGS | METH_KEYWORDS,
    "Cluster using TSP-graph & Ward's method, using python provided distance function."},   
    {NULL, NULL, 0, NULL}};

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0)*PyArray_STRIDES(py_v)[0] +                          \
                     (x1)*PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

/* This initiates the module using the above definitions. */
// #if PY_VERSION_HEX >= 0x03000000
static struct PyModuleDef moduledef = {
    PyModuleDef_HEAD_INIT, "tspg", NULL, -1, TSPgCluMethods, NULL, NULL, NULL, NULL};

PyMODINIT_FUNC PyInit_tspg(void) {
  PyObject *m;
  m = PyModule_Create(&moduledef);
  if (!m) {
    return NULL;
  }
  return m;
}

static PyObject *tspg_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyArrayObject *py_v;
  int num_neighbors = 10, num_clusters = 10, num_tsp = 10, maxiter = 100;
  float nndes_start = 0.0, endcond = 0.05;
  char *type = NULL;
  char *distance = NULL;
  int dfunc = D_L2;

  PyObject *ret;
  static char *kwlist[] = {"v", "num_clusters", "num_tsp", "dtype", "distance", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O!i|iss", kwlist, &PyArray_Type, &py_v,
                                   &num_clusters, &num_tsp, &type, &distance)) {
    return NULL;
  }

  if (num_tsp <= 0) {
    PyErr_SetString(PyExc_ValueError, "num_tsp <= 0 ");
  }

  printf("v %f %f\n", v(0, 0), v(0, 1));

  int dtype = D_L2;
  if (distance != NULL) {
    if (strcmp("l2", distance) == 0) {
      dtype = D_L2;
    } else if (strcmp("l1", distance) == 0) {
      dtype = D_L1;
    } else if (strcmp("cos", distance) == 0) {
      dtype = D_COS;
    } else {
      PyErr_SetString(PyExc_ValueError, "Distance must be one for {l2(default),l1,cos}");
      return NULL;
    }
  }

  printf("tspg_py 0000\n");

  ret = py_TSPgClu(py_v, num_clusters, num_tsp, dfunc);

  return ret;
}

static PyObject *tspg_generic_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyObject *py_v;
  int num_tsp = 5, num_clusters = 10;
  // char *type = NULL;
  // char *distance = NULL;
  // int dfunc = D_L2;

  PyObject *ret;
  static char *kwlist[] = {"v", "num_clusters", "num_tsp", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi|i", kwlist, &py_v, &num_clusters,
                                   &num_tsp)) {
    return NULL;
  }

  ret = py_TSPgCluGeneric(py_v, num_clusters, num_tsp);

  return ret;
}

} // END extern "C"
