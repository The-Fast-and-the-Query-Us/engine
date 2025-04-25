#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <utility>

namespace fast {
template <typename T> class linked_list {
private:
  struct node {
    T val;
    node *prev, *next;

    template <typename... args>
    node(args &&..._args)
        : val(std::forward<args>(_args)...), prev(nullptr), next(nullptr) {}

    node() : val(), prev(nullptr), next(nullptr) {}
  };

  node *head, *tail;
  size_t len;

public:
  class iterator {
    friend class linked_list<T>;
    node *current;

  public:
    iterator(node *_curr = nullptr) : current(_curr) {}

    T &operator*() const { return current->val; }
    T *operator->() const { return &current->val; }

    iterator &operator++() {
      current = current->next;
      return *this;
    }
    iterator operator++(int) {
      iterator tmp = *this;
      ++*this;
      return tmp;
    }

    iterator &operator--() {
      current = current->prev;
      return *this;
    }
    iterator operator--(int) {
      iterator tmp = *this;
      --*this;
      return tmp;
    }

    bool operator==(const iterator &other) const {
      return current == other.current;
    }
    bool operator!=(const iterator &other) const {
      return current != other.current;
    }
  };

  linked_list() : head(new node{}), tail(new node{}), len(0) {
    tail->prev = head;
    head->next = tail;
  }

  iterator begin() { return iterator(head->next); }

  iterator end() { return iterator(tail); }

  T &front() { return head->next->val; }

  T &back() { return tail->prev->val; }

  void push_front(const T &ele) {
    node *first = head->next;
    node *pushed = new node(ele);
    pushed->next = first;
    pushed->prev = head;
    head->next = pushed;
    first->prev = pushed;

    ++len;
  }

  void push_back(const T &ele) {
    node *last = tail->prev;
    node *pushed = new node(ele);
    pushed->next = tail;
    pushed->prev = last;
    tail->prev = pushed;
    last->next = pushed;

    ++len;
  }

  template <typename... args> iterator emplace_front(args &&..._args) {
    node *emplcd = new node(std::forward<args>(_args)...);

    emplcd->next = head->next;
    emplcd->prev = head;
    head->next->prev = emplcd;
    head->next = emplcd;

    ++len;
    return iterator(emplcd);
  }

  void pop_front() {
    if (len == 0) {
      return;
    }
    node *del = head->next;
    head->next = del->next;
    del->next->prev = head;
    delete del;
    --len;
  }

  void pop_back() {
    if (len == 0) {
      return;
    }
    node *del = tail->pev;
    tail->prev = del->prev;
    del->prev->next = tail;
    delete del;
    --len;
  }

  iterator erase(iterator victim) {
    if (victim == end())
      throw std::runtime_error("list erase out of bounds");

    node *to_delete = victim.current;

    iterator ret{to_delete->next};

    to_delete->next->prev = to_delete->prev;
    to_delete->prev->next = to_delete->next;

    --len;
    delete to_delete;

    return ret;
  }

  size_t size() { return len; }
};
} // namespace fast
