#pragma once

#include <cstddef>
#include <cstdint>
#include <string.hpp>
#include <pair.hpp>

namespace fast::query {
  constexpr size_t MAX_RESULTS = 10;
  typedef pair<uint32_t, string> Result;
}
