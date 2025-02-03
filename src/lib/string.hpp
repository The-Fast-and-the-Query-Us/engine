#pragma once

#include <cstddef>

namespace fast {

class string {
  size_t len;
  size_t cap;
  char* buffer;

  public:
  size_t size() const { return len; }

  char& operator[](size_t idx) const {
    return buffer[idx];
  }

  void operator+=(const string& str) {
    if(len + str.len >= cap) {
      grow(len + str.len + 1);
    }

    for()
  }
};

}
