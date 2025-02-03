#pragma once

#include <cstdint>

using hash_t = uint32_t;

namespace common {
  hash_t Hash(const char* word);
}
