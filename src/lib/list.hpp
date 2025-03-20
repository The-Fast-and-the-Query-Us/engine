#pragma once

#include <cstddef>
#include <common.hpp>
#include <cstdint>
#include <cstdlib>

#include <utility>
#include <new>

namespace fast {

/*
* A minimal singly linked list optimized for memory at the expense of features
* Overhead = 2 * sizeof(void*) + 1 bit per element
*/
template <class T, unsigned chunk_size=64>
class list {

  struct node {
    T arr[chunk_size];
    uintptr_t ptr; // tracks length for last and next for others
  };

  node* first;
  node* last;
  size_t len;

  // ensure that there is room in *last for one element
  inline void ensure_space() {
    if (last->ptr == chunk_size) {
      last->ptr = reinterpret_cast<uintptr_t>(malloc(sizeof(node)));
      last = reinterpret_cast<node*>(last->ptr);
      last->ptr = 0;
    }
  }

  public:

  list() {
    first = reinterpret_cast<node*>(malloc(sizeof(node)));
    last = first;
    first->ptr = 0;
  };

  list(const list& other) {
    first = reinterpret_cast<node*>(malloc(sizeof(node)));

    node *dst = first;
    node *src = other.first;

    while (src != other.last) {
      for (auto i = 0u; i < chunk_size; ++i) {
        new (dst->arr + i) T(src->arr[i]);
      }

      src = reinterpret_cast<node*>(src->ptr);
      dst->ptr = reinterpret_cast<uintptr_t>(malloc(sizeof(node)));
      dst = reinterpret_cast<node*>(dst->ptr);
    }

    for (auto i = 0u; i < src->ptr; ++i) {
      new (dst->arr + i) T(src->arr[i]);
    }

    dst->ptr = src->ptr;
    last = dst;

    len = other.len;
  }

  list& operator=(const list& other) {
    // could be optimized further to reuse space
    if (this != &other) {
      list tmp(other);
      swap(tmp.first, first);
      swap(tmp.last, last);
      swap(tmp.len, len);
    }
    return *this;
  }

  ~list() {
    while (first != last) {
      for (auto i = 0u; i < chunk_size; ++i) {
        first->arr[i].~T();
      }
      auto next = reinterpret_cast<node*>(first->ptr);
      free(first);
      first = next;
    }

    for (auto i = 0u; i < first->ptr; ++i) {
      first->arr[i].~T();
    }
    free(first);
  }

  void push_back(const T &element) {
    ensure_space();
    new (last->arr + last->ptr) T(element);
    ++last->ptr;
    ++len;
  }

  template <typename ... Args>
  void emplace_back(Args&&... args) {
    ensure_space();
    new (last->arr + last->ptr) T(std::forward<Args>(args)...);
    ++last->ptr;
    ++len;
  }

  T& back() const {
    return last->arr[last->ptr - 1];
  }

  size_t size() const { return len; }

  class iterator {
    node* node_;
    size_t offset_;


    public:
    iterator(node* node_, size_t offset_) : node_(node_), offset_(offset_) {}

    T& operator*() const { return node_->arr[offset_]; }

    iterator& operator++() {
      if (++offset_ == chunk_size) {
          offset_ = 0;
          node_ = reinterpret_cast<node*>(node_->ptr);
      }
      return *this;
    }

    bool operator!=(const iterator &other) const {
      return (
        node_ != other.node_ ||
        offset_ != other.offset_
      );
    }

    bool operator==(const iterator &other) const {
      return (
        node_ == other.node_ &&
        offset_ == other.offset_
      );
    }
  };

  iterator begin() const {
    return iterator(first, 0);
  }

  iterator end() const {
    if (last->ptr == chunk_size) {
      return iterator(reinterpret_cast<node*>(last->ptr), 0);
    } else {
      return iterator(last, last->ptr);
    }
  }
};

}
