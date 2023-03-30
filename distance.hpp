

struct nnGraph;

namespace tspg {
class Distance {
public:
  // Need to implement allways:
  virtual float operator()(int i, int j) const { return 0; };
  Distance(){};
  // Size needs to be defined allways:
  int size;
  int dimensionality;

  // Need to implement if mean_calculation=true;
  // get i_dim dimension value of i_vec vector
  virtual float getVecValue(int i_vec, int i_dim) { return 0; };
};

class GraphDistance : public Distance {
public:
  float **dm;
  nnGraph *graph;
  DataSet *data;
  int ncodes;
  // GraphDistance(nnGraph *graph_) : graph(graph_) { N = graph->size; }
  GraphDistance(nnGraph *graph_, DataSet *data_, int ncodes);
  ~GraphDistance();
  // float operator()(int i, int j) const __attribute__((noinline));
  float operator()(int i, int j) const;
};

class L2df : public Distance {
public:
  DataSet *data;
  L2df(DataSet *data_) : data(data_) {
    size = data->size;
    dimensionality = data->dimensionality;
  };
  float getVecValue(int i_vec, int i_dim) { return data->data[i_vec][i_dim]; };
  float operator()(int i, int j) const;
};

class L1df : public Distance {
public:
  DataSet *data;
  L1df(DataSet *data_) : data(data_) {
    size = data->size;
    dimensionality = data->dimensionality;
  };
  float getVecValue(int i_vec, int i_dim) { return data->data[i_vec][i_dim]; };
  float operator()(int i, int j) const;
};

// float cosine_dist(float *p1_idx, float *p2_idx, int D) {
// float dist = 0;
// float a_sum = 0;
// float b_sum = 0;
// float product = 0;
// for (int i = 0; i < D; i++) {
// product += (*p1_idx) * (*p2_idx);
// a_sum += (*p1_idx) * (*p1_idx);
// b_sum += (*p2_idx) * (*p2_idx);
// p1_idx++;
// p2_idx++;
// }
// dist = 1 - product * 1.0 / (sqrt(a_sum) * sqrt(b_sum));
// return dist;
// }

class EditDistance : public Distance {
  // const M &m;
public:
  DataSet *data;
  // GraphDistance(nnGraph *graph_) : graph(graph_) { N = graph->size; }
  EditDistance(DataSet *data_) : data(data_) { size = data->size; };

  // ~EditDistance();
  float operator()(int i, int j) const;
};

// Implementation copied from:
// https://en.wikibooks.org/wiki/Algorithm_Implementation/Strings/Levenshtein_distance#C++

float EditDistance::operator()(int a, int b) const {
  // unsigned int edit_distance2(const std::string &s1, const std::string &s2) {
  /*std::cout <<"s1:" << s1 << " s2:" << s2 << "\n";*/
  string s1 = data->strings->at(a);
  string s2 = data->strings->at(b);
  const std::size_t len1 = s1.size(), len2 = s2.size();
  std::vector<std::vector<unsigned int>> d(len1 + 1, std::vector<unsigned int>(len2 + 1));

  d[0][0] = 0;
  for (unsigned int i = 1; i <= len1; ++i)
    d[i][0] = i;
  for (unsigned int i = 1; i <= len2; ++i)
    d[0][i] = i;

  for (unsigned int i = 1; i <= len1; ++i)
    for (unsigned int j = 1; j <= len2; ++j)
      // note that std::min({arg1, arg2, arg3}) works only in C++11,
      // for C++98 use std::min(std::min(arg1, arg2), arg3)
      d[i][j] = std::min(
          {d[i - 1][j] + 1, d[i][j - 1] + 1, d[i - 1][j - 1] + (s1[i - 1] == s2[j - 1] ? 0 : 1)});
  return d[len1][len2];
}

} // namespace tspg
