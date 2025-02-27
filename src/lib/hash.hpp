#pragma once

#include <cstdint>

namespace fast {

enum HashType {
  RollingPoly
};

template <HashType hashtype = RollingPoly>
uint64_t hash(const char *word);

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

}
