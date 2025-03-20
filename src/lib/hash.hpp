#pragma once

#include "string_view.hpp"
#include <cstdint>

namespace fast {

enum HashType {
  RollingPoly
};

template <HashType hashtype = RollingPoly>
uint64_t hash(const char *word);

template <HashType hashtype = RollingPoly>
uint64_t hash(const string_view &word);


template <>
inline uint64_t hash<RollingPoly>(const char *word) {
  const uint64_t P = 101; 

  uint64_t res = 0, ppow = 1;

  for (auto i = word; *i; ++i) {
    res += (*i * ppow);
    ppow *= P;
  }

  return res;
}

template<>
inline uint64_t hash<RollingPoly>(const string_view &word) {
  const uint64_t P = 101; 

  uint64_t res = 0, ppow = 1;

  for (auto c : word) {
    res += (c * ppow);
    ppow *= P;
  }

  return res;
}

}
