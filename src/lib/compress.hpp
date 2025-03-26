#pragma once

#include "common.hpp"

#include <bit>
#include <cstddef>
#include <cstdint>
#include <cstring>

namespace fast {

enum class EncodeMethod {
  Simple3,
  Utf,
};

static constexpr EncodeMethod default_em = EncodeMethod::Utf;

template<EncodeMethod em = default_em>
unsigned char *encode(uint64_t num, unsigned char *buffer);

template<EncodeMethod em = default_em>
const unsigned char *decode(uint64_t &num, const unsigned char *buffer);

template<EncodeMethod em = default_em>
unsigned encoded_size(uint64_t num);

template<EncodeMethod em = default_em>
const unsigned char *skip_encoded(const unsigned char *buffer);

// Simple-3

template<>
inline unsigned char *encode<EncodeMethod::Simple3>(uint64_t num, unsigned char *buffer) {
  static_assert(std::endian::native == std::endian::little, "Compression not valid for non-little endian");
  const auto width = bit_width(num) + 3; // 3 header bits
  const auto extraBytes = (width - 1) / 8; // round down
  *buffer = (extraBytes << 5) | (num >> (extraBytes * 8));
  memcpy(buffer + 1, reinterpret_cast<char*>(&num), extraBytes);
  return buffer + extraBytes + 1;
}

template<>
inline const unsigned char *decode<EncodeMethod::Simple3>(uint64_t &num, const unsigned char *buffer) {
  static_assert(std::endian::native == std::endian::little, "Compression not valid for non-little endian");
  const unsigned char extra = (*buffer) >> 5;
  num = uint64_t((*buffer) & 0x1F) << (8 * extra);
  memcpy(reinterpret_cast<char*>(&num), buffer + 1, extra);
  return buffer + extra + 1;
}

template<>
inline unsigned encoded_size<EncodeMethod::Simple3>(uint64_t num) {
  static_assert(std::endian::native == std::endian::little, "Compression not valid for non-little endian");
  return (
    (bit_width(num) + 3 + 7) / 8 // plus 7 to round up
  );
}

template<>
inline const unsigned char *skip_encoded<EncodeMethod::Simple3>(const unsigned char *buffer) {
  static_assert(std::endian::native == std::endian::little, "Compression not valid for non-little endian");
  return buffer + ((*buffer) >> 5) + 1;
}

// UTF : high bit is end of seq, stored MSB to LSB

template<>
inline unsigned char *encode<EncodeMethod::Utf>(uint64_t num, unsigned char *buffer) {
  while (num > 0x7F) {
    *buffer = num & 0x7F;
    num >>= 7;
    ++buffer;
  }
  *buffer = num | 0x80;
  return buffer + 1;
}

template<>
inline const unsigned char *decode<EncodeMethod::Utf>(uint64_t &num, const unsigned char *buffer) {
  size_t count = 0;
  num = 0;
  while (*buffer < 0x80) {
    num |= uint64_t(*buffer) << 7 * count++;
    ++buffer;
  }
  num |= uint64_t(*buffer & 0x7F) << 7 * count;
  return buffer + 1;
}

template<>
inline unsigned encoded_size<EncodeMethod::Utf>(uint64_t num) {
  return (bit_width(num) + 6) / 7;
}

template<>
inline const unsigned char *skip_encoded<EncodeMethod::Utf>(const unsigned char *buffer) {
  while (*(buffer++) < 0x80);
  return buffer;
}

}
