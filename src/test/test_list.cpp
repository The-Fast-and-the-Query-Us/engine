#include <cassert>
#include <list.hpp>

using namespace fast;

void test_seq(const fast::list<int> &l, const int N) {
  int i = 0;
  for (auto it = l.begin(); it != l.end(); ++i, ++it) assert(*it == i);

  assert(i == N);

  i = 0;
  for (const auto num : l) assert(num == i++);

  assert(i == N);

  if (N > 0) assert(l.back() == N - 1);
}

void run_test(const list<int> &l, const int N) {
  test_seq(l, N);
  const auto other(l);
  test_seq(other, N);
  list<int> another;
  another.emplace_back(1);
  another = other;
  test_seq(another, N);
}

int main() {

  list<int> l;

  for (auto i = 0; i < 64 * 2; ++i) {
    if (i % 32 == 0) run_test(l, i);
    l.push_back(i);
  }

}
