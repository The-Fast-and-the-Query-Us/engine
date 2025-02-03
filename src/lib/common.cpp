#include "common.h"

namespace common {

/*
 * rolling polynomial hash
 */
hash_t Hash(const char* word) {
  const hash_t P = 101;
  hash_t hash = 0;
  hash_t ppow = 1;

  for (; *word; ++word) {
    hash += (*word * ppow);
    ppow *= P;
  }

  return hash;
}
}
