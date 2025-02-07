// Templated queue
#pragma once
#include "exception.hpp"
#include <algorithm>
#include <cstddef>
#include <cstdlib>
#include <cstring>

namespace fast {

template <typename T> class queue {
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
    sz = other.sz;
    cap = other.cap;
    l = other.l;
    r = other.r;
    delete[] buf;
    buf = new T[cap];
    std::memcpy(buf, other.buf, sz);
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
      throw(exception("Front on empty queue\n"));
    return buf[l];
  }

  T &back() {
    if (!sz)
      throw(exception("Front on empty queue\n"));
    return buf[r];
  }

  void push(const T &e) {
    if (cap == sz) {
      grow();
      l = 0, r = sz - 1;
    }
    r = (r + 1) % sz;
    buf[r] = e;
    ++sz;
  }

  void pop() {
    r = (r - 1 + sz) % sz;
    --sz;
  }

private:
  size_t sz, cap, l, r;
  T *buf;

  static inline size_t grow_sz(size_t old) {
    return std::max(old << 1, size_t(2));
  }

  void grow() {
    cap = grow_sz(cap);
    T *newbuf = new T[cap];
    size_t tmp = sz - l;
    std::memcpy(newbuf, buf + l, tmp);
    std::memcpy(newbuf + tmp - 1, buf, l);

    delete[] buf;
    buf = newbuf;
  }
};
} // namespace fast
