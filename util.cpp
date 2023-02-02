
#include "util.hpp"

void handler(int sig) {
  void *array[10];
  size_t size;

  // get void*'s for all entries on the stack
  size = backtrace(array, 10);

  // print out all the frames to stderr
  fprintf(stderr, "Error: signal %d:\n", sig);
  backtrace_symbols_fd(array, size, STDERR_FILENO);
  exit(1);
}

void __terminal_error(const char *file, const unsigned line, const char *func, const char *msg) {

  printf("[%s:%u:%s] %s\n", file, line, func, msg);
  fflush(stdout);
  /*std::raise(SIGINT);*/
  exit(1);
}


inline int rand_int(int max_int) { return std::rand() % (max_int); }
inline int rand_int_range(int min_int, int max_int) {
  return min_int + (std::rand() % (max_int - min_int + 1));
}

static void *safe_malloc(size_t n, unsigned int line) {
  void *p = malloc(n);
  if (!p) {
    fprintf(stderr, "[%s:%u]Out of memory(%u bytes)\n", __FILE__, line, (unsigned int)n);
    exit(EXIT_FAILURE);
  }
  return p;
}

inline int *rand_int_vector(int max_int, int n) {
  int *int_v = (int *)safemalloc(sizeof(int) * n);
  for (int i = 0; i < n; i++) {
    int_v[i] = rand_int(max_int + 1);
  }
  return int_v;
}

int count_lines(FILE *fp) {
  int num_lines = 0;
  char ch;
  fseek(fp, 0L, SEEK_SET);
  while (!feof(fp)) {
    ch = fgetc(fp);
    if (ch == '\n') {
      num_lines++;
    }
  }
  fseek(fp, 0L, SEEK_SET);
  return num_lines;
}

void write_ints_to_fp(FILE *fp, int *data, int N) {
  int i;
  for (int i = 0; i < N; i++) {
    fprintf(fp, "%d", data[i]);
    fprintf(fp, "\n");
  }
}

void write_ints_to_file(const char *fn, int *data, int N) {
  int i;

  FILE *fp;
  fp = fopen(fn, "w");
  for (int i = 0; i < N; i++) {
    fprintf(fp, "%d", data[i]);
    fprintf(fp, "\n");
  }
  fclose(fp);
}

void write_flt_vec2_to_file(const char *fn, vector<vector<float>> *vec2) {
  int i;
  FILE *fp;
  fp = fopen(fn, "w");

  vector<vector<float>>::iterator row;
  vector<float>::iterator col;
  for (row = (*vec2).begin(); row != (*vec2).end(); row++) {
    for (col = row->begin(); col != row->end(); col++) {
      if (col != row->begin()) {
        fprintf(fp, " ");
      }
      fprintf(fp, "%f", (*col));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}


void write_dbl_vec2_to_file(const char *fn, vector<vector<double>> *vec2) {
  int i;
  FILE *fp;
  fp = fopen(fn, "w");

  vector<vector<double>>::iterator row;
  vector<double>::iterator col;
  for (row = (*vec2).begin(); row != (*vec2).end(); row++) {
    for (col = row->begin(); col != row->end(); col++) {
      if (col != row->begin()) {
        fprintf(fp, " ");
      }
      fprintf(fp, "%f", (*col));
    }
    fprintf(fp, "\n");
  }
  fclose(fp);
}

