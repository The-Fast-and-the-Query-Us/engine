#pragma once

#include <common.hpp>

#include <compare>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace fast {

class string {
  char *start_ = nullptr;
  size_t len_;
  size_t cap;

  void grow(size_t need) {
    start_ = static_cast<char*>(realloc(start_, need + 1));
    cap = need;
  }

 public:
  string() {
    grow(8);
    len_ = 0;
    start_[len_] = 0;
  }

  string(const string& other) {
    grow(other.cap);
    len_ = other.len_;
    memcpy(start_, other.start_, len_ + 1); // +1 for null
  }

  string(const char *cstr) {
    size_t str_len{0};
    for (auto ptr = cstr; *ptr; ++ptr, ++str_len);

    grow(str_len);
    len_ = str_len;
    memcpy(start_, cstr, len_ + 1);
  }

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
    if (need > cap) grow(need);
  }

  void operator+=(char c) {
    if (len_ == cap) grow(len_ << 1);
    start_[len_++] = c;
    start_[len_] = 0;
  }

  void operator+=(const string& other) {
    if (len_ + other.len_ > cap) grow(len_ + other.len_);
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

  char &operator[](size_t idx) { return start_[idx]; }

  char *begin() const { return start_; }

  char *end() const { return start_ + len_; }

  size_t size() const { return len_; }

  bool operator==(const string &other) const {
    return (
      len_ == other.len_ &&
      memcmp(start_, other.start_, len_) == 0
    );
  }

  std::strong_ordering operator<=>(const string &other) const {
    const auto cmp = memcmp(start_, other.start_, min(len_, other.len_));

    if (cmp < 0)      return std::strong_ordering::less;
    else if (cmp > 0) return std::strong_ordering::greater;
    else              return len_ <=> other.len_;
  }
};

}
