#pragma once

#include <cstddef>

namespace fast {

template <class T, size_t size=64>
class list {
  struct node {
    T arr[size];
    size_t len = 0;
    node* next = nullptr;
  };

  node* first;
  node* last;
  size_t len;

  public:
  list() : first(new node), last(first), len(0) {};

  size_t length() { return len; }

  void push_back(T element) {
    if (last->len == size) {
      last->next = new node;
      last = last->next;
    }
    last->arr[last->len++] = element;
  }

  class iterator {
    node* node_;
    size_t offset_;
    public:
    iterator(node* node_, size_t offset_) : node_(node_), offset_(offset_) {}

    T& operator*() { return node_->arr[offset_]; }

    iterator& operator++() {
      if (++offset_ == node_->len) {
          offset_ = 0;
          node_ = node_->next;
      }
      return *this;
    }

    bool operator!=(const iterator& other) {
      return !(
        node_ == other.node_ &&
        offset_ == other.offset_
      );
    }
  };

  iterator begin() const {
    return iterator(first, 0);
  }

  iterator end() const {
    return iterator(nullptr, 0);
  }
};

}
