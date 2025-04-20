#pragma once

#include <cstddef>
#include <cstdlib>
#include <unistd.h>

namespace fast {

/*
* Use combination of insertion sort and quicksort for large arrays
*/
template<class T>
void fast_sort(T* begin, T* end) {
  const size_t N = end - begin;

  if (N <= 16) {
    for (size_t i = 1; i < N; ++i) {
      const auto tmp = begin[i];
      size_t j;

      for (j = i; j > 0 && begin[j] < begin[j - 1]; --j) {
        begin[j] = begin[j - 1];
      }

      begin[j] = tmp;
    }
  } else {
    swap(begin[rand() % N], end[-1]);

    auto i = begin;

    for (auto j = begin; j < end - 1; ++j) {
      if (*j < end[-1]) {
        swap(*i, *j);
        ++i;
      }
    }

    swap(*i, end[-1]);

    fast_sort(begin, i);
    fast_sort(i + 1, end);
  }
}

}
