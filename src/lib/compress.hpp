#pragma once

#include "common.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace fast {

/*
 * Encode Simple-3
 */
inline unsigned char *encode(uint64_t num, unsigned char *buffer) {
  const auto width = bit_width(num) + 3; // 3 header bits
  const auto extraBytes = (width - 1) / 8; // round down

  *buffer = (extraBytes << 5) | (num >> (extraBytes * 8));

  if constexpr (std::endian::native == std::endian::little) {
    num = __builtin_bswap64(num);
  }

  memcpy(buffer + 1, reinterpret_cast<char*>(&num + 1) - extraBytes, extraBytes);
  return buffer + extraBytes + 1;
}

/*
 * Decode Simple-3
 */
inline const unsigned char *decode(uint64_t &num, const unsigned char *buffer) {
  const unsigned char extra = (*buffer) >> 5;

  num = 0;
  memcpy(reinterpret_cast<char*>(&num + 1) - extra, buffer + 1, extra);

  if constexpr (std::endian::native == std::endian::little) {
    num = __builtin_bswap64(num);
  }

  num |= uint64_t((*buffer) & 0x1F) << (8 * extra);

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

/*
* Return the number of bytes required to compress the given number
*/
template<class T>
size_t compressed_size(T number) {
  size_t ans{1};
  number >>= 7;
  while (number) number >>= 7, ++ans;
  return ans;
}

/*
* Write a number to a buffer with high bits coming first
*/
template<class T>
char* compress(T number, char* buffer) {
  for (auto byte = compressed_size(number) - 1; byte; --byte, ++buffer) {
    *buffer = (number >> (7 * byte)) | 0x80;
  }

  *buffer = (number) & ~(0x80);

  return buffer + 1;
}

/*
* Read bytes writen by compress
*/
template <class T>
char* expand(T& number, char* buffer) {
  number = 0;
  do {
    number <<= 7;
    number |= uint8_t(*buffer & ~(0x80));
  } while (*(buffer++) & 0x80);
  return buffer;
}

// skip to one after current compressed number
inline char* skip_compressed(char *buffer) {
  while (*(buffer++) & 0x80);
  return buffer;
}

}
