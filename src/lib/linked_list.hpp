#include <cstdint>

namespace fast {
template <typename T> class linked_list {
public:
  linked_list()
      : head(new node{T{}, nullptr, nullptr}),
        tail(new node{T{}, tail, nullptr}), len(0) {
    head->next = tail;
  }

  void push_front(T ele) {
    node *first = head->next;
    node *pushed = new node{ele, head, first};
    head->next = pushed;
    first->prev = pushed;

    ++len;
  }

  void push_back(T ele) {
    node *last = tail->prev;
    node *pushed = new node{ele, last, tail};
    tail->prev = pushed;
    last->next = pushed;

    ++len;
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

  size_t size() { return len; }

private:
  struct node {
    T val;
    node *prev, *next;
  };

  node *head, *tail;
  size_t len;
};
} // namespace fast
