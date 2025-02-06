#pragma once
#include <cstddef>
#include <cstring>

namespace fast {
class static_string {
  char *buffer_;
  size_t len_;

  public:
  static_string() : buffer_(nullptr), len_(0) {}

  static_string(const char *start, const char *end) {
    len_ = end - start;
    buffer_ = new char[len_];
    memcpy(buffer_, start, len_);
  }

  static_string(const char *word) {
    len_ = 0;
    for (auto ptr = word; *ptr; ++ptr, ++len_);

    buffer_ = new char[len_];
    memcpy(buffer_, word, len_);
  }

  static_string(const static_string& other) {
    len_ = other.len_;
    buffer_ = new char[len_];

    memcpy(buffer_, other.buffer_, len_);
  }

  static_string& operator=(const static_string& other) {
    if (this != &other) {
      delete [] buffer_;
      len_ = other.len_;
      buffer_ = new char[len_];
      memcpy(buffer_, other.buffer_, len_);
    }
    return *this;
  }

  ~static_string() {
    delete[] buffer_;
  }

  bool operator==(const static_string &other) const {
    if (len_ != other.len_) return false;

    for (auto i = 0ul; i < len_; ++i)
      if (buffer_[i] != other.buffer_[i]) return false;

    return true;
  }

  char* begin() const { return buffer_; }
  char* end() const { return buffer_ + len_; }
};
}
