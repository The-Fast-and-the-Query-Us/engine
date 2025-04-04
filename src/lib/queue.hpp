// Templated queue
#pragma once
#include "exception.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace fast {

template <typename T>
class queue {
public:
  queue() : sz(0), cap(0), l(0), r(0), buf(nullptr) {}

  queue(const queue &other)
      : sz(other.sz), cap(other.cap), l(other.l), r(other.r),
        buf(new T[other.cap]) {
    std::memcpy(buf, other.buf, sz);
  }

  queue(queue &&other)
      : sz(other.sz), cap(other.cap), l(other.l), r(other.r), buf(other.buf) {
    other.buf = nullptr;
    other.sz = other.cap = other.l = other.r = 0;
  }

  queue &operator=(const queue &other) {
    if (other.buf != buf) {
      sz = other.sz;
      cap = other.cap;
      l = other.l;
      r = other.r;
      delete[] buf;
      buf = new T[cap];
      std::memcpy(buf, other.buf, sz);
    }
    return *this;
  }

  queue &operator=(queue &&other) {
    sz = other.sz;
    cap = other.cap;
    l = other.l;
    r = other.r;
    delete[] buf;
    buf = other.buf;
    other.buf = nullptr;
    other.sz = other.cap = other.l = other.r = 0;
    return *this;
  }

  ~queue() { delete[] buf; }

  T &front() {
    if (!sz)
      throw(fast::exception("Front on empty queue\n"));
    return buf[l];
  }

  T &back() {
    if (!sz)
      throw(fast::exception("Front on empty queue\n"));
    return buf[(r + cap - 1) % cap];
  }

  void push(const T &e) {
    if (cap == sz) {
      grow();
    }
    buf[r] = e;
    r = (r + 1) % cap;
    ++sz;
  }

  void pop() {
    l = (l + 1) % cap;
    --sz;
  }

  size_t size() { return sz; }

  inline bool empty() { return sz == 0; }

  void print() {
    std::cout << "index:\t";
    for (size_t i = 0; i < cap; ++i) {
      std::cout << i << "\t";
    }
    std::cout << '\n';
    std::cout << "value:\t";
    for (size_t i = 0; i < cap; ++i) {
      std::cout << buf[i] << "\t";
    }
    std::cout << '\n';
    std::cout << "l/r: \t";
    for (size_t i = 0; i < cap; ++i) {
      std::cout << (i == l ? 'l' : (i == r ? 'r' : ' ')) << "\t";
    }
    std::cout << '\n';
  }

private:
  size_t sz, cap, l, r;
  T *buf;

  static inline size_t grow_sz(size_t old) {
    return std::max(old << 1, size_t(2));
  }

  void grow() {
    size_t first_part = cap - l;
    cap = grow_sz(cap);

    T *newbuf = new T[cap];

    for (size_t i = 0; i < first_part; ++i) {
      newbuf[i] = buf[l + i];
    }
    for (size_t i = 0; i < r; ++i) {
      newbuf[first_part + i] = buf[i];
    }

    l = 0;
    r = sz;

    delete[] buf;
    buf = newbuf;
  }
};
} // namespace fast
