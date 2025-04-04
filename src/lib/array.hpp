#pragma once

#include <cstddef>

namespace fast {

template <class T, size_t len>
class array {
  T data[len]{};
  public:

  size_t size() const { return len; }

  T* begin()             { return data; }
  const T* begin() const { return data; }

  T* end()              { return data + len; }
  const T* end()  const { return data + len; }

  T &operator[](size_t idx) { return data[idx]; }
  const T &operator[](size_t idx) const { return data[idx]; }
};

}
