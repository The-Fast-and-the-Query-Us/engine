#pragma once
#include <cstddef>

namespace fast {
class static_string {
  char *buffer_;
  size_t len_;

  public:
  static_string(const char *start, const char *end);

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
