#pragma once

#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string_view.hpp>

namespace fast {

class string : public string_view {
  size_t cap;

  void grow(size_t need) {
    start_ = (char*) realloc(start_, need + 1);
    cap = need;
  }

 public:
  string() {
    start_ = static_cast<char*>(malloc(9));
    cap = 8;
    len_ = 0;

    start_[len_] = 0;
  }

  string(const string& other) {
    cap = other.cap;
    len_ = other.len_;
    start_ = static_cast<char*>(malloc(cap + 1));
    memcpy(start_, other.start_, len_ + 1); // +1 for null
  }

  ~string() { free(start_); }


  const char *c_str() const { return begin(); }

  void reserve(size_t need) {
    if (need > cap) grow(need);
  }

  void operator+=(char c) {
    if (len_ == cap) grow(len_ << 1);
    start_[len_++] = c;
  }

  void operator+=(const string& other) {
    if (len_ + other.len_ > cap) grow(len_ + other.len_);
    memcpy(start_ + len_, other.start_, other.len_);
    len_ += other.len_;
  }

  string operator+(const string &rhs) {
    string ans(*this);
    ans += rhs;
    return ans;
  }
};

}
