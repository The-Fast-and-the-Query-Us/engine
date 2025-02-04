#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace fast {

class string {
  size_t len;
  size_t cap;
  char* buffer;

  void grow(size_t need) {
    buffer = (char*) realloc(buffer, need + 1);
    cap = need;
  }

 public:
  string() : len(0), cap(7), buffer((char*) malloc(8)) {}

  size_t size() const { return len; }

  char& operator[](size_t idx) const { return buffer[idx]; }

  const char *c_str() const { return data(); }

  void reserve(size_t need) {
    if (need > cap) grow(need);
  }

  char *data() const { 
    buffer[len] = 0;
    return buffer; 
  }

  void push_back(char c) {
    if (len == cap) grow(len << 1);
    buffer[len++] = c;
  }

  void push_back(const string& other) {
    if (len + other.len > cap) grow(len + other.len);
    memcpy(buffer + len, other.buffer, other.len);
    len += other.len;
  }
};

}
