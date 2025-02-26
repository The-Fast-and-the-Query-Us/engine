#pragma once

#include <cstddef>
#include <cstdint>
#include <cstring>

namespace fast {

template <class T>
void swap(T &lhs, T &rhs) {
  T tmp = lhs;
  lhs = rhs;
  rhs = tmp;
}

template <class ... Args>
struct tuple;

template <>
struct tuple<>{};

template <class Head, class ... Tail>
struct tuple<Head, Tail...> {

  constexpr tuple(Head h, Tail... t) : head(h), tail(t...) {}
  constexpr tuple() {}

  Head head;
  tuple<Tail...> tail;
};

template <size_t i, class Head, class... Tail>
constexpr auto& get(tuple<Head, Tail...> &t) {
  if constexpr (i == 0) return t.head;
  else return get<i - 1>(t.tail);
}

/*
 * Return smallest x such that x >= base && x % mult == 0
 */
inline size_t round_up(size_t base, size_t mult) {
  return (base + (mult - 1)) & ~(mult - 1);
}

// return number of bits needed to hold number n 
// ie 1 past the index of the highest bit
inline uint8_t bit_width(uint64_t n) {
  return (n == 0) ? 1 : 64 - __builtin_clzll(n);
}

// return floor(sqrt(n))
// Made fast by lack of division operations
inline uint64_t fast_sqrt(const uint64_t n) {
  auto shift = bit_width(n);
  shift += shift & 1; // round up to power of 2

  auto ans = 0ull;
  do {
    shift -= 2;
    ans <<= 1;
    ans |= 1; // guess bit
    ans ^= ans * ans > (n >> shift); // check if too big
  } while (shift != 0);

  return ans;
}

template <class T>
inline void write_unaligned(const T &t, char *buffer) {
  memcpy(buffer, &t, sizeof(t));
}

template <class T>
inline T read_unaligned(const char *buffer) {
  T t;
  memcpy(&t, buffer, sizeof(t));
  return t;
}

template <class T>
T max(T lhs, T rhs) {
  return (lhs < rhs) ? rhs : lhs;
}

template <class T>
T min(T lhs, T rhs) {
  return (lhs < rhs) ? lhs : rhs;
}

}
