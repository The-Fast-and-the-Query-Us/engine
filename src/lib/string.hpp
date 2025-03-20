#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <optional>

namespace fast {

class string {
  size_t len;
  size_t cap;
  char *buffer;

  void grow(size_t need) {
    buffer = (char *)realloc(buffer, need + 1);
    cap = need;
  }

public:
  string() : len(0), cap(7), buffer((char *)malloc(8)) {}

  string(const string &other) {
    len = other.len;
    cap = other.len;
    buffer = (char *)malloc(cap + 1);

    memcpy(buffer, other.buffer, len);
  }

  string(const char *str) {
    len = strlen(str);
    cap = strlen(str);
    buffer = (char *)malloc(cap + 1);

    memcpy(buffer, str, len);
  }

  ~string() { free(buffer); }

  size_t size() const { return len; }

  char &operator[](size_t idx) const { return buffer[idx]; }

  const char *c_str() const { return data(); }

  char *data() const {
    buffer[len] = 0;
    return buffer;
  }

  void reserve(size_t need) {
    if (need > cap)
      grow(need);
  }

  void reverse(size_t l, size_t r) {
    for (; l < (l + r) / 2; ++l) {
      char temp = buffer[l];
      buffer[l] = buffer[r];
      buffer[r--] = temp;
    }
  }

  bool operator==(const string &s) const {
    if (s.size() != this->size())
      return false;

    for (size_t i = 0; i < s.size(); i++) {
      if (s[i] != (*this)[i])
        return false;
    }

    return true;
  }

  bool operator!=(string &s) { return !(*this == s); }

  void operator+=(char c) {
    if (len == cap)
      grow(len << 1);
    buffer[len++] = c;
  }

  void operator+=(const string &other) {
    if (len + other.len > cap)
      grow(len + other.len);
    memcpy(buffer + len, other.buffer, other.len);
    len += other.len;
  }

  string operator+(const string &rhs) {
    string ans(*this);
    ans += rhs;
    return ans;
  }
};

} // namespace fast
