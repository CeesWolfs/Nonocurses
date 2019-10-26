#include <algorithm>
#include <bitset>
#include <iostream>
#include <vector>

#include "vector2d.h"

constexpr int MAX = 25;

typedef enum : char { EMPTY = 'x', FULL = '#', UNKOWN = '.' } cell;

bool next_permutation(int N, std::bitset<MAX>& mask,
                      const std::vector<int>& hints) {
  int start_pos = hints[0];
  // Start at the first
  int i = start_pos;
  int hint = 1;
  while (true) {
    // find next bit set
    while (i < N && mask[i] != 1) i++;
    // if we there are spaces at the end, there are no more permutations
    if (i >= N) return false;
    // if distance is more than one swap and return permutation
    if ((i - start_pos) > 1) {
      // shift the pattern one down
      mask[i - 1] = 1;
      mask[i + hints[hint] - 1] = 0;
      return true;
    }
    start_pos = i + hints[hint] + 1;
    i = start_pos;
    hint++;
  }
}

bool build_masks(int n, const std::vector<int>& hints,
                 Vector2D<std::bitset<MAX>>& masks) {
  // get number of spaces between
  masks.new_vector();
  int sum = 0;
  for (auto& n : hints) sum += n;
  int spaces = n - sum - (hints.size() - 1);
  int pos = 0;
  std::bitset<MAX> mask = 0;
  if (sum == 0) {
    masks.push_back(mask);
    return true;
  }
  mask = ((1 << hints[0]) - 1);
  if (hints.size() == 1) {
    masks.push_back(mask);
    return true;
  }
  pos += hints[0] + spaces + 1;
  for (int i = 1; i < hints.size(); ++i) {
    mask |= ((1 << hints[i]) - 1) << pos;
    pos += hints[i] + 1;
  }
  masks.push_back(mask);
  while (next_permutation(n, mask, hints)) masks.push_back(mask);
  return true;
}

class nonogram {
 private:
  Vector2D<std::bitset<MAX>> row_masks;
  Vector2D<std::bitset<MAX>> col_masks;
  int unknowns() {
    // returns the number of still unknown cells
    int bitcounts = 0;
    for (auto a : fixed_rows) {
      bitcounts += a.count();
    }
    return (n_rows * n_cols - bitcounts);
  }

  std::pair<int, int> unknownCell() {
    // find an uknown cell
    int row = 0;
    for (auto b : fixed_rows) {
      if (b.count() >= n_rows) {
        row++;
        continue;
      }
      for (int i = 0; i < n_cols; i++) {
        if (!b.test(i)) {
          std::make_pair(row, i);
        }
      }
      row++;
    }
    return std::make_pair(-1, -1);
  }

 public:
  const std::vector<std::vector<int>> h_hints;
  const std::vector<std::vector<int>> v_hints;

  int n_rows;
  int n_cols;

  std::vector<std::bitset<MAX>> rows;
  std::vector<std::bitset<MAX>> cols;
  std::vector<std::bitset<MAX>> fixed_rows;
  std::vector<std::bitset<MAX>> fixed_cols;

  nonogram(const std::vector<std::vector<int>> h_hints,
           const std::vector<std::vector<int>> v_hints)
      : row_masks(h_hints.size()),
        col_masks(v_hints.size()),
        h_hints(h_hints),
        v_hints(v_hints) {
    // init sizes and memory
    n_rows = h_hints.size();
    n_cols = v_hints.size();

    rows = std::vector<std::bitset<MAX>>(n_rows);
    cols = std::vector<std::bitset<MAX>>(n_cols);
    fixed_rows = std::vector<std::bitset<MAX>>(n_rows);
    fixed_cols = std::vector<std::bitset<MAX>>(n_cols);

    for (int i = 0; i < n_rows; ++i) {
      // init mask arrrays and nonograms
      build_masks(n_cols, h_hints[i], row_masks);
      rows[i] = row_masks[i][0];
    }
    for (int i = 0; i < n_cols; ++i) {
      build_masks(n_rows, v_hints[i], col_masks);
      cols[i] = col_masks[i][0];
    }
    std::cout << row_masks[0][0] << '\n';
  }
  ~nonogram() {}

  int solve() {
    // solve the nonogram, returns number of valid solutions
    int solutions = 0;
    int deductions = 0;
    int unknowns = this->unknowns();
    while (unknowns > 0) {
      int prev_unkowns = unknowns;
      for (int i = 0; i < n_rows; ++i) {
        deductions = deduce_row(i);
        if (deductions < 0) return 0;
        std::cout << *this << '\n';
        unknowns -= deductions;
      }
      for (int i = 0; i < n_cols; ++i) {
        deductions = deduce_col(i);
        if (deductions < 0) return 0;
        std::cout << *this << '\n';
        unknowns -= deductions;
      }
      if (unknowns == prev_unkowns) {
        // guess a cell and divert
        auto [row, col] = unknownCell();
        fixCell(col, row, 1);
        solutions += solve();
        fixCell(col, row, 0);
        solutions += solve();
      }
    }
    // we found a solution!
    return std::max(1, solutions);
  }

  inline void setCell(int x, int y, bool val) {
    rows[y][x] = val;
    cols[x][y] = val;
  }
  inline void fixCell(int col, int row, bool val) {
    fixed_rows[row][col] = 1;
    fixed_cols[col][row] = 1;
    rows[row][col] = val;
    cols[col][row] = val;
  }

  inline void unfixCell(int col, int row) {
    fixed_rows[row][col] = 0;
    fixed_cols[col][row] = 0;
  }

  int deduce_col(int col) {
    // deduce cells in a column, returns number of newly found cells
    if (fixed_cols[col].count() >= n_rows) return 0;
    auto line = cols[col];

    bool one_fits = false;

    // todo: check if no masks fit
    // first fix all positions
    std::bitset<MAX> new_fixed((1 << n_rows) - 1);
    for (auto mask : col_masks[col]) {
      // shift value of mask
      auto shift = mask;
      do {
        // if the current mask doesn't fit the fixed places, skip
        if (((shift ^ line) & fixed_cols[col]) != 0) {
          shift <<= 1;
          continue;
        }
        // else unfix all changed positions but only first time
        cols[col] = shift;
        if (one_fits) new_fixed &= (line ^ shift).flip();
        shift <<= 1;
        one_fits = true;
      } while (shift.to_ulong() < (1 << n_rows) && shift != 0);
    }
    int determined = new_fixed.count() - fixed_cols[col].count();
    fixed_cols[col] = new_fixed;
    for (int row = 0; row < n_rows; ++row) {
      fixed_rows[row][col] = new_fixed[row];
      rows[row][col] = cols[col][row];
    }
    if (!one_fits) return -1;
    return determined;
  }

  int deduce_row(int row) {
    if (fixed_rows[row].count() >= n_cols) return 0;
    auto line = rows[row];
    bool one_fits = false;

    // todo: check if no masks fit
    // first fix all positions
    std::bitset<MAX> new_fixed((1 << n_cols) - 1);
    for (auto mask : row_masks[row]) {
      // shift value of mask
      auto shift = mask;
      do {
        // if the current mask doesn't fit the fixed places, skip
        if (((shift ^ line) & fixed_rows[row]) != 0) {
          shift <<= 1;
          continue;
        }
        one_fits = true;
        // else unfix all changed positions
        rows[row] = shift;
        if (one_fits) new_fixed &= (line ^ shift).flip();
        shift <<= 1;
      } while (shift.to_ulong() < (1 << n_cols) && shift != 0);
    }
    int determined = new_fixed.count() - fixed_rows[row].count();
    fixed_rows[row] = new_fixed;
    for (int col = 0; col < n_cols; ++col) {
      fixed_cols[col][row] = new_fixed[col];
      cols[col][row] = rows[row][col];
    }
    if (!one_fits) return -1;
    return determined;
  }
  void to_str() {}

  void print() { std::cout << this; }
  friend std::ostream& operator<<(std::ostream& os, const nonogram& nono);
};

std::ostream& operator<<(std::ostream& os, const nonogram& nono) {
  char buffer[nono.n_cols + 1];
  buffer[nono.n_cols] = '\0';
  for (int row = 0; row < nono.n_rows; ++row) {
    for (int i = 0; i < nono.n_cols; ++i) {
      if (!nono.fixed_rows[row][i]) {
        buffer[i] = '?';
      } else if (nono.rows[row][i]) {
        buffer[i] = '#';
      } else {
        buffer[i] = '.';
      }
    }
    os << buffer;
    for (auto h : nono.h_hints[row]) {
      os << h << ' ';
    }
    os << '\n';
  }
  bool more_hints = true;
  int depth = 0;
  while (more_hints) {
    more_hints = false;
    for (int col = 0; col < nono.n_cols; ++col) {
      if ((nono.v_hints[col].size() - 1) < depth) {
        os << ' ';
      } else {
        os << nono.v_hints[col][depth];
        more_hints = true;
      }
    }
    depth++;
    os << '\n';
  }

  return os;
}