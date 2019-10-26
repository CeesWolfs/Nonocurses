#include <ncurses.h>
#include <stdio.h>

#include <algorithm>
#include <iterator>
#include <sstream>

#include "nonogram.h"

static WINDOW *create_newwin(int height, int width, int starty, int startx) {
  WINDOW *local_win = newwin(height, width, starty, startx);
  box(local_win, 0, 0);
  keypad(local_win, TRUE);

  return (local_win);
}

void print_nonogram(WINDOW *my_win, const nonogram &nono) {
  int win_x;
  int win_y;
  getyx(my_win, win_y, win_x);
  char buffer[nono.n_cols];
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
    mvwaddnstr(my_win, row + 1, 1, buffer, nono.n_cols);
    move(win_y + row + 2, win_x + nono.n_cols + 3);
    for (auto h : nono.h_hints[row]) {
      printw("%d ", h);
    }
  }
  for (int col = 0; col < nono.n_cols; ++col) {
    std::ostringstream oss;

    if (!nono.v_hints[col].empty()) {
      // Convert all but the last element to avoid a trailing ","
      std::copy(nono.v_hints[col].begin(), nono.v_hints[col].end() - 1,
                std::ostream_iterator<int>(oss, " "));

      // Now add the last element with no delimiter
      oss << nono.v_hints[col].back();
    }
    int y = win_y + nono.n_rows + 3;
    for (char c : oss.str()) {
      mvaddch(y++, win_x + col + 2, c);
    }
  }
}

int main(int argc, char **argv) {
  WINDOW *my_win;

  int ch;
  int x = 0;
  int y = 0;
  const int win_x = 2;
  const int win_y = 2;
  nonogram nono(
      {{3, 3},
       {5, 5},
       {13},
       {13},
       {13},
       {13},
       {11},
       {9},
       {7},
       {5},
       {3},
       {1},
       {0}},
      {{4}, {6}, {8}, {9}, {10}, {10}, {10}, {10}, {10}, {9}, {8}, {6}, {4}});

  initscr();
  // start_color();
  cbreak();
  keypad(stdscr, TRUE);
  noecho();

  // init_pair(1, COLOR_RED, COLOR_BLACK);
  // init_pair(2, COLOR_WHITE, COLOR_BLACK);
  // attron(COLOR_PAIR(2));

  printw("NonoEs: 1 leip nonogram spelletje");
  mvprintw(1, 0,
           "[q]uit [f]ill [u]nfill [x]onbekend [c]heck [r]deduce row [d]educe "
           "column");

  refresh();
  my_win = create_newwin(nono.n_rows + 2, nono.n_cols + 2, win_y, win_x);
  wmove(my_win, 1, 1);
  print_nonogram(my_win, nono);
  refresh();

  while ((ch = wgetch(my_win)) != 'q') {
    switch (ch) {
      case KEY_LEFT:
        x--;
        break;
      case KEY_RIGHT:
        x++;
        break;
      case KEY_UP:
        y--;
        break;
      case KEY_DOWN:
        y++;
        break;
      case 'f':
        waddch(my_win, '#');
        nono.fixCell(x - 1, y - 1, true);
        break;
      case 'o':
        waddch(my_win, '.');
        nono.fixCell(x - 1, y - 1, false);
        break;
      case 'u':
        waddch(my_win, '?');
        nono.unfixCell(x - 1, y - 1);
        break;
      case 'r':
        nono.deduce_row(y - 1);
        print_nonogram(my_win, nono);
        break;
      case 'd':
        nono.deduce_col(x - 1);
        print_nonogram(my_win, nono);
        break;

      default:
        break;
    }
    y = std::max(1, y);
    y = std::min(y, nono.n_rows);
    x = std::max(1, x);
    x = std::min(x, nono.n_cols);
    wmove(my_win, y, x);
    refresh();
  }

  endwin();
  return 0;
}