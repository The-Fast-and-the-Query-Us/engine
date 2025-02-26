#pragma once
#include <compare>
#include <cstddef>
#include <cstring>
#include <common.hpp>

namespace fast {
/*
* A non-owning, lightweight string class
* !All other string class should inherit from this!
*/
class string_view {

protected:

  char *start_;
  char *end_;

public:

  string_view(char *start, char *end) : start_(start), end_(end) {}
  
  string_view(char *cstr) {
    start_ = cstr;
    while (*cstr) ++cstr;
    end_ = cstr;
  }

  char *begin() const { return start_; }

  char *end() const { return end_; }

  size_t length() const { return end_ - start_; }

  // requires i < length()
  char &operator[](size_t i) const { return start_[i]; }

  std::strong_ordering operator<=>(const string_view &other) const {
    const auto cmp = memcmp(start_, other.start_, min(length(), other.length()));
    
    if (cmp < 0)      return std::strong_ordering::less;
    else if (cmp > 0) return std::strong_ordering::greater;
    else              return length() <=> other.length();
  }
};

}
