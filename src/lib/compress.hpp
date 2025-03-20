#pragma once

#include "common.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace fast {

static_assert(std::endian::native == std::endian::little, "Compression not valid for non-little endian");

/*
 * Encode Simple-3
 *  num must be bitwidth(num) <= 61
 */
inline unsigned char *encode(uint64_t num, unsigned char *buffer) {
  const auto width = bit_width(num) + 3; // 3 header bits
  const auto extraBytes = (width - 1) / 8; // round down

  *buffer = (extraBytes << 5) | (num >> (extraBytes * 8));

  memcpy(buffer + 1, reinterpret_cast<char*>(&num), extraBytes);
  return buffer + extraBytes + 1;
}

/*
 * Decode Simple-3
 */
inline const unsigned char *decode(uint64_t &num, const unsigned char *buffer) {
  const unsigned char extra = (*buffer) >> 5;

  num = uint64_t((*buffer) & 0x1F) << (8 * extra);
  memcpy(reinterpret_cast<char*>(&num), buffer + 1, extra);

  return buffer + extra + 1;
}

/*
 * Size of num when in Simple-3 Encoding
 */
inline unsigned encoded_size(uint64_t num) {
  return (
    (bit_width(num) + 3 + 7) / 8 // plus 7 to round up
  );
}

/*
 * Jump to next Simple-3 encoded value
 */
inline const unsigned char *skip_encoded(const unsigned char *buffer) {
  return buffer + ((*buffer) >> 5) + 1;
}

}
