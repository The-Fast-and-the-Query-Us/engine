#pragma once
#include <compare>
#include <cstddef>
#include <cstring>
#include <common.hpp>

namespace fast {

/*
* A non-owning, lightweight string class
*/
class string_view {

protected:

  char *start_;
  size_t len_;

public:

  string_view() : start_(nullptr), len_(0) {}

  string_view(const char *start, const char *end) : start_(const_cast<char*>(start)), len_(end - start) {}

  string_view(const char *start, size_t len) : start_(const_cast<char*>(start)), len_(len) {}
  
  string_view(const char *cstr) { // TODO Optimize
    start_ = const_cast<char*>(cstr);
    len_ = 0;
    while (*cstr) ++cstr, ++len_;
  }

  const char *begin() const { return start_; }

  const char *end() const { return start_ + len_; }

  size_t length() const { return len_; }

  // requires i < length()
  const char &operator[](size_t i) const { return start_[i]; }

  std::strong_ordering operator<=>(const string_view &other) const {
    const auto cmp = memcmp(start_, other.start_, min(length(), other.length()));
    
    if (cmp < 0)      return std::strong_ordering::less;
    else if (cmp > 0) return std::strong_ordering::greater;
    else              return len_ <=> other.len_;
  }

  bool operator==(const string_view &other) const {
    return (
      len_ == other.len_ &&
      memcmp(start_, other.start_, len_) == 0
    );
  }
};

}
