#pragma once

#include <cstddef>

namespace fast {

class string {
  size_t len;
  size_t cap;
  char* buffer;

  void grow(size_t new_len) {
    // buffer = rea
  }

 public:
  string() : len(0), cap(8), buffer(new char[8]) {}

  size_t size() const { return len; }

  char& operator[](size_t idx) const { return buffer[idx]; }

  void operator+=(const string& str) {
    size_t new_len = len + str.len + 1;
    if (new_len > cap) {
      grow(new_len);
    }

    for (size_t i = 0; i < str.len; ++i) {
      buffer[len + i] = str.buffer[i];
    }

    len = new_len;
  }
};

}  // namespace fast
