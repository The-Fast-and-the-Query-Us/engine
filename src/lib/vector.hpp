// vector.h
//
// Starter file for a vector template

#pragma once

#include <cassert>
#include <common.hpp>
#include <cstddef>  // for size_t
#include <cstdlib>
#include <new>

namespace fast {

template <typename T>
class vector {
 public:
  // Default Constructor
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Constructs an empty vector with capacity 0
  vector() : elts{nullptr}, size_{0}, cap_{0} {}

  ~vector() {
    clear();
    free(elts);
  }

  vector(size_t num_elements) : size_{num_elements} {
    grow(num_elements);
    for (auto i = 0u; i < num_elements; ++i) new (elts + i) T();
  }

  vector(size_t num_elements, const T &val) : size_{num_elements} {
    grow(num_elements);
    for (auto i = 0u; i < num_elements; ++i) new (elts + i) T(val);
  }

  vector(const vector<T> &other) : size_{other.size_} {
    grow(other.size_);
    for (auto i = 0u; i < size_; ++i) new (elts + i) T(other.elts[i]);
  }

  vector operator=(const vector<T> &other) {
    if (this != &other) {
      clear();
      if (other.size_ > cap_) grow(other.size_);
      for (auto i = 0u; i < other.size_; ++i) {
        new (elts + i) T(other.elts[i]);
      }
      size_ = other.size_;
    }
    return *this;
  }

  vector(vector<T> &&other) {
    elts = other.elts;
    cap_ = other.cap_;
    size_ = other.size_;

    other.elts = nullptr;
    other.cap_ = 0;
    other.size_ = 0;
  }

  vector operator=(vector<T> &&other) {
    if (this != &other) {
      swap(elts, other.elts);
      swap(cap_, other.cap_);
      swap(size_, other.size_);
    }
    return *this;
  }

  void reserve(size_t newCapacity) {
    if (newCapacity > cap_) grow(newCapacity);
  }

  void resize(size_t newSize) {
    if (newSize > size_) {
      grow(newSize);

      for (size_t i = 0; i < cap_; i++)
        new (elts + i) T();

      size_ = newSize;
    }
  }

  size_t size() const { return size_; }

  size_t capacity() const { return cap_; }

  T &operator[](size_t i) { return elts[i]; }

  const T &operator[](size_t i) const { return elts[i]; }

  T &back() { return elts[size_ - 1]; }

  T &front() { return elts[0]; }

  void push_back(const T &x) {
    if (size_ == cap_) {
      grow(max(size_t(8), cap_ << 1));
    }
    new (elts + size_++) T(x);
  }

  template <typename... Args>
  void emplace_back(Args &&...args) {
    if (size_ == cap_) grow(max(size_t(8), cap_ << 1));
    new (elts + size_++) T(args...);
  }

  void pop_back(size_t count = 1) {
    for (auto i = 1u; i <= count; ++i) elts[size_ - i].~T();
    size_ -= count;
  }

  T *begin() { return elts; }

  T *end() { return elts + size_; }

  const T *begin() const {
    return elts;
  }

  const T *end() const {
    return elts + size_;
  }

  void clear() {
    for (auto i = 0u; i < size_; ++i) elts[i].~T();
    size_ = 0;
  }

 private:
  T *elts = nullptr;
  size_t size_ = 0;
  size_t cap_ = 0;

  void grow(size_t new_cap) {
    elts = static_cast<T *>(
        realloc(static_cast<void *>(elts), new_cap * sizeof(T)));
    cap_ = new_cap;
  }
};
}  // namespace fast
