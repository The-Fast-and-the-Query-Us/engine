#pragma once

#include <cstddef>

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

}
