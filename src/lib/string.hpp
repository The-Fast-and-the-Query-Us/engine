#pragma once

#include "string_view.hpp"
#include <cassert>
#include <compare>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace fast {

/*
 *  Add SVO by using pointer to store string?
 */
class string {
  char *start_ = nullptr;
  size_t len_;
  size_t cap;

  void grow(size_t need) {
    start_ = static_cast<char *>(realloc(start_, need + 1));
    assert(start_ != nullptr);
    cap = need;
  }

public:
  string() {
    grow(8);
    len_ = 0;
    start_[len_] = 0;
  }

  string(const string &other) {
    grow(other.cap);
    len_ = other.len_;
    memcpy(start_, other.start_, len_ + 1); // +1 for null
  }

  string(const char *cstr) { // maybe mark explicit to avoid accidentaly heap
                             // allocation?
    size_t str_len{0};
    for (auto ptr = cstr; *ptr; ++ptr, ++str_len)
      ;

    grow(str_len);
    len_ = str_len;
    memcpy(start_, cstr, len_ + 1);
  }

  string(const char *begin, size_t len) {
    grow(len);
    len_ = len;
    memcpy(start_, begin, len);
    start_[len_] = 0;
  }

  string(const char *begin, const char *end) : string(begin, end - begin) {}

  string(const string_view &sv) : string(sv.begin(), sv.size()) {}

  string &operator=(const string &other) {
    if (this != &other) {
      grow(other.len_);
      len_ = other.len_;
      memcpy(start_, other.start_, other.len_ + 1);
    }
    return *this;
  }

  ~string() { free(start_); }

  const char *c_str() const { return start_; }

  void reserve(size_t need) {
    if (need > cap)
      grow(need);
  }

  void resize(size_t size, char fill = 'a') {
    reserve(size);
    for (auto i = len_; i < size; ++i) {
      start_[i] = fill;
    }
    len_ = size;
    start_[len_] = 0;
  }

  void reverse(size_t l, size_t r) {
    for (; l < (l + r) / 2; ++l) {
      char temp = *(start_ + l);
      *(start_ + l) = *(start_ + r);
      *(start_ + r--) = temp;
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
    if (len_ == cap)
      grow(len_ << 1);
    start_[len_++] = c;
    start_[len_] = 0;
  }

  void operator+=(const string &other) {
    if (len_ + other.len_ > cap)
      grow(len_ + other.len_);
    memcpy(start_ + len_, other.start_, other.len_);
    len_ += other.len_;
    start_[len_] = 0;
  }

  string operator+(const string &rhs) {
    string ans(*this);
    ans += rhs;
    return ans;
  }

  // requires that count <= len_
  void pop_back(size_t count = 1) {
    len_ -= count;
    start_[len_] = 0;
  }

  string substr(size_t i, size_t len) { return string(start_ + i, len); }

  char &operator[](size_t idx) const { return start_[idx]; }

  char *begin() const { return start_; }

  char *end() const { return start_ + len_; }

  size_t size() const { return len_; }

  operator string_view() const { return string_view(start_, len_); }

  string_view view() const { return this->operator string_view(); }

  template<class T>
  bool operator==(const T &other) const {
    return this->view() == static_cast<string_view>(other);
  }

  template<class T>
  std::strong_ordering operator<=>(const T &other) const {
    return this->view() <=> static_cast<string_view>(other);
  }

  bool operator==(const char *cstr) const {
    return this->view() == cstr;
  }

  std::strong_ordering operator<=>(const char *cstr) const {
    return this->view() <=> cstr;
  }
};

} // namespace fast
