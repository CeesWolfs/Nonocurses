#include <vector>

template <typename T>
class PseudoVector {
 public:
  class iterator {
   public:
    iterator(T *ptr) : ptr(ptr) {}
    iterator operator++() {
      ++ptr;
      return *this;
    }
    bool operator!=(const iterator &other) const { return ptr != other.ptr; }
    const T &operator*() const { return *ptr; }

   private:
    T *ptr;
  };

 private:
  unsigned len;
  T *val;

 public:
  PseudoVector(T *ptr, unsigned size) {
    val = ptr;
    len = size;
  }
  T &operator[](int index) {
    if (index >= len) throw "Array index out of bounds";
    return val[index];
  }
  unsigned size() { return len; }
  iterator begin() const { return iterator(val); }
  iterator end() const { return iterator(val + len); }
};

// 2D Vector structure with arbiturary sized 1D subvectors that doesn't fragment
// memory
template <typename T>
class Vector2D {
 private:
  std::vector<T> data;
  std::vector<unsigned> indices;

 public:
  Vector2D(int N) {
    // make sure we reserve space for N vectors and 4 elements per vector on
    // average (arbituary)
    indices.reserve(N);
    data.reserve(N * 4);
  }

  // Overloading [] operator to access elements in array style
  PseudoVector<T> operator[](int row) {
    if (row >= indices.size()) throw "Vector2D index out of bounds";
    unsigned index = indices[row];
    int next_row = row + 1;
    unsigned next_index;
    if (indices.size() <= next_row) {
      next_index = data.size();
    } else {
      next_index = indices[next_row];
    }
    return PseudoVector<T>(data.data() + index, next_index - index);
  }
  //  add an element to the current last vector
  void push_back(T val) { data.push_back(val); }
  // add new 1D vector
  void new_vector() { indices.push_back(data.size()); }
  // clear
  void clear() {
    data.clear();
    indices.clear();
  }
};