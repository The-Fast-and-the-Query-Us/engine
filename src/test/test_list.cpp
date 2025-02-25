#include <cassert>
#include <list.hpp>

constexpr int SIZE = 10'000;

void test_seq(fast::list<int> &l) {
  int i = 0;
  for (auto it = l.begin(); it != l.end(); ++i, ++it) assert(*it == i);

  i = 0;
  for (const auto num : l) assert(num == i++);
}

int main() {
  fast::list<int> l;
  for (int i = 0; i < SIZE; ++i) {
    l.push_back(i);
    assert(l.back() == i);
  }

  test_seq(l);

  fast::list cpy(l);
  test_seq(cpy);

  l = cpy;
  test_seq(l);

  fast::list<int> empty;
  for (const auto _ : empty) {
    (void)_;
    assert(false);
  }
}
