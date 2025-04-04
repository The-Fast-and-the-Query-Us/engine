#pragma once

#include <cstddef>
#include <cstdint>
#include <pair.hpp>
#include <string.hpp>

namespace fast::query {
constexpr size_t MAX_RESULTS = 10;
typedef pair<double, string> Result;
}  // namespace fast::query
