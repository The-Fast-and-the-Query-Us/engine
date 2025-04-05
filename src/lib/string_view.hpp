#pragma once

#include <cassert>
#include <compare>
#include <cstddef>
#include <cstring>

#include "common.hpp"

namespace fast {

class string_view {
  const char *start;
  size_t len;

 public:
  string_view() : start(nullptr), len(0) {}

  string_view(const char *start, size_t len) : start(start), len(len) {}

  string_view(const char *start, const char *end)
      : start(start), len(end - start) {}

  string_view(const char *cstr) : start(cstr), len(0) {
    for (auto it = start; *it; ++it, ++len);
  }

  size_t size() const { return len; }

  char operator[](size_t i) const { return start[i]; }

  const char *begin() const { return start; }
  const char *end() const { return start + len; }

  const char *find(const char *to_find) const {
    if (!*to_find) return start;

    size_t find_len = 0;
    while (to_find[find_len]) ++find_len;

    for (size_t i = 0; i < len; ++i) {
      bool match = true;

      for (size_t j = 0; j < find_len; j++) {
        if (i + j >= len || start[i + j] != to_find[j]) {
          match = false;
          break;
        }
      }

      if (match) {
        return &start[i];  // found match
      }
    }

    return nullptr;  // no match found
  }

  string_view sub_view(size_t start, size_t length) {
    // this should get compiled out in release mode
    assert(start + length <= this->size() && 
           "sub_view would go past the end of the main view");
    return string_view(this->start + start, length);
  }

  string_view trim_suffix(size_t count) {
    return sub_view(0, size() - count);
  }

  string_view trim_prefix(size_t count) {
    return sub_view(count, size() - count);
  }

  bool operator==(const string_view &other) const {
    return (len == other.len && memcmp(start, other.start, len) == 0);
  }

  std::strong_ordering operator<=>(const string_view &other) const {
    const auto cmp = memcmp(start, other.start, min(len, other.len));

    if (cmp < 0)
      return std::strong_ordering::less;
    else if (cmp > 0)
      return std::strong_ordering::greater;
    else
      return len <=> other.len;
  }

  bool operator==(const char *cstr) const {  // do we allow null to be in view?
    for (size_t i = 0; i < len; ++i) {
      if (start[i] != cstr[i]) return false;
    }
    return !cstr[len];
  }

  std::strong_ordering operator<=>(const char *cstr) const {
    for (size_t i = 0; i < len; ++i) {
      if (start[i] < cstr[i])
        return std::strong_ordering::less;
      else if (start[i] > cstr[i])
        return std::strong_ordering::greater;
    }

    if (cstr[len])
      return std::strong_ordering::less;
    else
      return std::strong_ordering::equal;
  }
};
}  // namespace fast
