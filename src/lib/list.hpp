#pragma once

#include <cstddef>
#include <common.hpp>

namespace fast {

template <class T, size_t size=64>
class list {
  // maybe use allign as for better cache performance
  struct node {
    T arr[size]; // use malloc to not call element constructors
    size_t len = 0; // we could optimize by using pointer to track len if not supporting iterator operations
    node* next = nullptr;
  };

  node* first;
  node* last;
  size_t len;

  public:
  list() : first(new node), last(first), len(0) {};

  list(const list& other) {
    auto ptr = other.first; 
    node** dst = &first;
    len = other.len;

    while (ptr) {
      *dst = new node;
      for (auto i = 0u; i < ptr->len; ++i) {
        (*dst)->arr[i] = ptr->arr[i];
      }
      (*dst)->len = ptr->len;
      last = *dst;

      dst = &((*dst)->next);
      ptr = ptr->next;
    }

    *dst = nullptr;
  }

  list& operator=(const list& other) {
    if (this != &other) {
      list temp(other);
      swap(first, temp.first);
      swap(last, temp.last);
      swap(len, temp.len);
    }
    return *this;
  }

  ~list() {
    while (first) {
      const auto next = first->next;
      delete first;
      first = next;
    }
  }

  size_t length() const { return len; }

  void push_back(T element) {
    if (last->len == size) {
      last->next = new node;
      last = last->next;
    }
    last->arr[last->len++] = element;
    ++len;
  }

  T* back() const {
    return &(last->arr[last->len - 1]);
  }

  class iterator {
    node* node_;
    size_t offset_;
    public:
    iterator(node* node_, size_t offset_) : node_(node_), offset_(offset_) {}

    T& operator*() const { return node_->arr[offset_]; }

    iterator& operator++() {
      if (++offset_ == node_->len) {
          offset_ = 0;
          node_ = node_->next;
      }
      return *this;
    }

    bool operator!=(const iterator& other) const {
      return (
        node_ != other.node_ ||
        offset_ != other.offset_
      );
    }
  };

  iterator begin() const {
    if (len == 0) return end();
    return iterator(first, 0);
  }

  iterator end() const {
    return iterator(nullptr, 0);
  }
};

}
