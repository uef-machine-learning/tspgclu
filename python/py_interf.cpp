

#include <Python.h>
#include <math.h>
#include <numpy/arrayobject.h>
#include <stdio.h>

#include <stdio.h>
#include "contrib/argtable3.h"

#ifdef defined(_POSIX_)
// #include <execinfo.h>
// #include <signal.h>
#include <unistd.h>
#endif

#include <stdlib.h>
#include <stdio.h>
#include <iostream>
#include <vector>

using namespace std;

struct stat2 {
  int num_calc_clu_dist;
  int num_pruned;
};

#include "tspg_lib.hpp"

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
#include <stdio.h>

using namespace std;

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0) * PyArray_STRIDES(py_v)[0] +                        \
                     (x1) * PyArray_STRIDES(py_v)[1])))
#define v_shape(i) (py_v->dimensions[(i)])

PyObject *array_to_py(int *arr, int N) {
  // Convert c array to python format
  // printf("array_to_py size=%d\n", N);
  PyObject *pyarr = PyList_New(N);
  for (int i = 0; i < N; i++) {
    PyList_SetItem(pyarr, i, Py_BuildValue("i", arr[i]));
  }
  return pyarr;
}

PyObject *merge_order_to_py(std::vector<std::vector<float>> mo) {
  // Convert c array to python format
  // printf("array_to_py size=%d\n", N);
  PyObject *pyarr = PyList_New(mo.size());
  for (int i = 0; i < mo.size(); i++) {
    PyObject *arr2 = PyList_New(4);
    PyList_SetItem(arr2, 0, Py_BuildValue("i", static_cast<int>(mo[i][0])));
    PyList_SetItem(arr2, 1, Py_BuildValue("i", static_cast<int>(mo[i][1])));
    PyList_SetItem(arr2, 2, Py_BuildValue("f", mo[i][2]));
    PyList_SetItem(arr2, 3, Py_BuildValue("i", static_cast<int>(mo[i][3])));
    PyList_SetItem(pyarr, i, arr2);
  }
  return pyarr;
}

PyObject *py_TSPgCluster(PyObject *py_v, int num_clusters, int num_tsp, int dfunc) {

  PyObject *ret;
  PyObject *py_labels;
  PyObject *py_peaks;
  printf("py_TSPgClu\n");
  g_timer.tick();

  int N = PyList_Size(py_v);
  PyObject *vec1 = PyList_GetItem(py_v, 0);
  int D = PyList_Size(vec1);
  printf("N=%d D=%d\n", N, D);
  DataSet *data = init_DataSet(N, D);

  for (int i = 0; i < N; i++) {
    PyObject *vec = PyList_GetItem(py_v, i);
    for (int j = 0; j < D; j++) {
      PyObject *pyval = PyList_GetItem(vec, j);
      float val = (float)PyFloat_AsDouble(pyval);
      set_val(data, i, j, val);

      if (i < 10) {
        printf(" %f", val);
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

  tspg::Distance *dfun = nullptr;
  dfun = dynamic_cast<tspg::Distance *>(new tspg::L2df(data));

  // if (distfunc->count > 0) {
  // if (strcmp(distfunc->sval[0], "l2") == 0) {
  // printf("Distance function: %s\n", distfunc->sval[0]);
  // g_options.mean_calculation = 1;
  // } else if (strcmp(distfunc->sval[0], "l1") == 0) {
  // printf("Distance function: %s\n", distfunc->sval[0]);
  // dfun = dynamic_cast<tspg::Distance *>(new tspg::L1df(data));
  // }
  // }

  TSPclu<tspg::Distance> tspgclu(num_clusters, num_tsp /*num_tsp*/, dfun, 1);

  centroids = new vector<vector<float>>(num_clusters, vector<float>(data->dimensionality, 0));
  // int *part = cluster_tspg(data, nng, num_clusters, centroids);
  int *part = tspgclu.runClustering();

  py_labels = array_to_py(part, N);
  PyObject *py_mo = merge_order_to_py(tspgclu.mergeOrder);

  // int arr2[] = {1, 2, 3, 4};
  ret = PyList_New(2);
  PyList_SetItem(ret, 0, py_labels);
  PyList_SetItem(ret, 1, py_mo);

  return ret;
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

  py_labels = array_to_py(part, N);

  return py_labels;
}

extern "C" {

static PyObject *tspg_py(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject *tspg_generic_py(PyObject *self, PyObject *args, PyObject *kwargs);
static PyObject *tspg_create_graph_py(PyObject *self, PyObject *args, PyObject *kwargs);

// Define python accessible methods
static PyMethodDef TSPgCluMethods[] = {

    {"tspg", (PyCFunction)tspg_py, METH_VARARGS | METH_KEYWORDS,
     "Cluster using TSP-graph & Ward's method."},
    {"create_graph", (PyCFunction)tspg_create_graph_py, METH_VARARGS | METH_KEYWORDS, "Create a TSP graph"},
    {"tspg_generic", (PyCFunction)tspg_generic_py, METH_VARARGS | METH_KEYWORDS,
     "Cluster using TSP-graph & Ward's method, using python provided distance function."},
    {NULL, NULL, 0, NULL}};

#define v(x0, x1)                                                                                  \
  (*(npy_float64 *)((PyArray_DATA(py_v) + (x0) * PyArray_STRIDES(py_v)[0] +                        \
                     (x1) * PyArray_STRIDES(py_v)[1])))
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

DataSet *pyToDataset(PyObject *py_v) {

  printf("pyToDataset\n");
  if (!PyList_Check(py_v)) {
    PyErr_SetString(PyExc_TypeError, "Input must be a list");
    return NULL;
  }

  int N = PyList_Size(py_v);
  PyObject *vec1 = PyList_GetItem(py_v, 0);
  int D = PyList_Size(vec1);
  printf("N=%d D=%d\n", N, D);
  DataSet *data = init_DataSet(N, D);

  for (int i = 0; i < N; i++) {
    PyObject *vec = PyList_GetItem(py_v, i);
    // if (!PyList_Check(vec)) {
    // PyErr_SetString(PyExc_TypeError, "Inner elements must be lists");
    // return NULL;
    // }

    for (int j = 0; j < D; j++) {
      PyObject *pyval = PyList_GetItem(vec, j);
      float val = (float)PyFloat_AsDouble(pyval);
      set_val(data, i, j, val);

      if (i < 10) {
        printf(" %f", val);
      }
    }
    if (i < 10) {
      printf("\n");
    }
  }
  return data;
}

static PyObject *tspg_create_graph_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyObject *py_v;
  int num_tsp = 10;
  char *type = NULL;
  char *distance = NULL;

  PyObject *ret;
  const char *kwlist[] = {"v", "num_tsp", "dtype", "distance", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "O|iss", const_cast<char**>(kwlist), &py_v, &num_tsp, &type,
                                   &distance)) {
    return NULL;
  }

  if (num_tsp <= 0) {
    PyErr_SetString(PyExc_ValueError, "num_tsp <= 0 ");
  }

  // printf("v %f %f\n", v(0, 0), v(0, 1));

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

  printf("tspg_create_graph_py\n");

  PyObject *col = PyList_New(num_tsp);
  DataSet *data = pyToDataset(py_v);
  if (data == NULL) {
    return NULL;
  }
  g_options.distance_type = data->distance_type = dtype;

  int *ind_arr = (int *)safemalloc(sizeof(int) * data->size);
  int *ind_arr2 = (int *)safemalloc(sizeof(int) * data->size);

  for (int i = 0; i < num_tsp; i++) {
    for (int i_data = 0; i_data < data->size; i_data++) {
      ind_arr[i_data] = i_data;
    }
    rpdiv_queue(data, ind_arr, ind_arr2);

    PyObject *arr1 = array_to_py(ind_arr, data->size);
    PyList_SetItem(col, i, arr1);
  }

  free(ind_arr);
  free(ind_arr2);

  return col;
}

static PyObject *tspg_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyObject *py_v;
  int num_neighbors = 10, num_clusters = 10, num_tsp = 10, maxiter = 100;
  char *type = NULL;
  char *distance = NULL;
  int dfunc = D_L2;

  PyObject *ret;
  // static const char *kwlist[] = {"v", "num_clusters", "num_tsp", "dtype", "distance", NULL};
  const char *kwlist[] = {"v", "num_clusters", "num_tsp", "dtype", "distance", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi|iss", const_cast<char **>(kwlist), &py_v,
                                   &num_clusters, &num_tsp, &type, &distance)) {
    return NULL;
  }

  if (num_tsp <= 0) {
    PyErr_SetString(PyExc_ValueError, "num_tsp <= 0 ");
  }

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

  printf("tspg_py 0020\n");

  ret = py_TSPgCluster(py_v, num_clusters, num_tsp, dfunc);

  return ret;
}

static PyObject *tspg_generic_py(PyObject *self, PyObject *args, PyObject *kwargs) {
  import_array();
  PyObject *py_v;
  int num_tsp = 5, num_clusters = 10;

  PyObject *ret;
  const char *kwlist[] = {"v", "num_clusters", "num_tsp", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, kwargs, "Oi|i", const_cast<char **>(kwlist), &py_v,
                                   &num_clusters, &num_tsp)) {
    return NULL;
  }

  ret = py_TSPgCluGeneric(py_v, num_clusters, num_tsp);

  return ret;
}

} // END extern "C"
