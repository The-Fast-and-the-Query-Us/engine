#pragma once

#include "common.hpp"
#include "flat_map.hpp"
#include "string_view.hpp"

#include <compare>
#include <cstddef>
#include <cstdlib>
#include <cstring>

#include "string_view.hpp"

namespace fast {

/*
 *  Add SVO by using pointer to store string?
 */

template <typename K, typename V> class flat_map;

class string {
  friend class flat_map<string, int>;
  char *start_ = nullptr;
  size_t len_;
  size_t cap;

  void grow(size_t need) {
    start_ = static_cast<char *>(realloc(start_, need + 1));
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
    while (l < r) {
      char temp = *(start_ + l);
      *(start_ + l) = *(start_ + r);
      *(start_ + r) = temp;
      l++;
      r--;
    }
  }

  void insert(size_t idx, char c) {
    if (idx > len_) {
      return;
    }
    if (len_ + 1 > cap) {
      grow((len_ + 1));
    }
    if (idx < len_) {
      memmove(start_ + idx + 1, start_ + idx, len_ - idx);
    }
    start_[idx] = c;
    len_++;
    start_[len_] = 0;
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

  void append(const char *other, size_t other_sz) {
    if (len_ + other_sz > cap)
      grow(len_ + other_sz);
    memcpy(start_ + len_, other, other_sz);
    len_ += other_sz;
    start_[len_] = 0;
  }

  void operator+=(const string &other) {
    if (len_ + other.len_ > cap)
      grow(len_ + other.len_);
    memcpy(start_ + len_, other.start_, other.len_);
    len_ += other.len_;
    start_[len_] = 0;
  }

  string operator+(const string &rhs) const {
    string ans(*this);
    ans += rhs;
    return ans;
  }

  string operator+(const char *other) const {
    string ans(*this), adtnl(other);
    ans += adtnl;
    return ans;
  }

  friend string operator+(const char *lhs, const string &rhs) {
    return string(lhs) + rhs;
  }

  // requires that count <= len_
  void pop_back(size_t count = 1) {
    len_ -= count;
    start_[len_] = 0;
  }

  // get substr of start until end of string
  string substr(size_t i) { return string{start_ + i, len_ - i}; }

  string substr(size_t i, size_t len) const {
    if (i >= len_)
      return string();
    if (len + i > len_)
      return string(*this);
    return string{start_ + i, len};
  }

  bool starts_with(const string_view &sv) const {
    if (sv.size() > len_)
      return false;

    for (size_t i = 0; i < sv.size(); ++i) {
      if (start_[i] != sv[i])
        return false;
    }

    return true;
  }

  bool ends_with(const string_view &sv) const {
    if (sv.size() > len_)
      return false;

    auto offset = len_ - sv.size();

    for (size_t i = 0; i < sv.size(); ++i) {
      if (start_[offset + i] != sv[i])
        return false;
    }

    return true;
  }

  // requires len_ > 0
  char back() const { return start_[len_ - 1]; }

  char &operator[](size_t idx) const { return start_[idx]; }

  // should be marked const char * but dont want to break anything yet
  char *begin() const { return start_; }

  // same as above
  char *end() const { return start_ + len_; }

  size_t size() const { return len_; }

  operator string_view() const { return string_view(start_, len_); }

  string_view view() const { return this->operator string_view(); }

  template <class T> bool operator==(const T &other) const {
    return this->view() == static_cast<string_view>(other);
  }

  template <class T> std::strong_ordering operator<=>(const T &other) const {
    return this->view() <=> static_cast<string_view>(other);
  }

  bool operator==(const char *cstr) const { return this->view() == cstr; }

  std::strong_ordering operator<=>(const char *cstr) const {
    return this->view() <=> cstr;
  }

  // find first index of c in string >= from
  // return -1 if not found
  ssize_t find(char c, size_t from = 0) const {
    for (size_t i = from; i < len_; ++i) {
      if (start_[i] == c)
        return i;
    }
    return -1;
  }

  bool contains(const string_view &word) const {
    if (len_ < word.size()) return false;

    for (size_t i = 0; i < len_ - word.size(); ++i) {

      size_t j;
      for (j = 0; j < word.size() && word[j] == start_[i + j]; ++j);

      if (j == word.size()) return true;

    }

    return false;
  }
};

inline string to_string(uint64_t num) {
  string res;

  if (num == 0)
    return "0";

  while (num) {
    res += '0' + (num % 10);
    num /= 10;
  }

  for (size_t i = 0; i < res.size() / 2; ++i) {
    swap(res[i], res[res.size() - 1 - i]);
  }

  return res;
}

} // namespace fast
