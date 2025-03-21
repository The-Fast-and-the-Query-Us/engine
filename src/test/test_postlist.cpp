#include "list.hpp"
#include "postlist.hpp"
#include "types.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <iostream>

using namespace fast;

void test(const list<post<Text>> &l) {
  const auto space = postlist<Text>::size_needed(l);
  auto pl = (postlist<Text>*) malloc(space);
  memset(pl, 0, space);
  postlist<Text>::write(l, pl);

  auto it = l.begin();
  for (const auto num : *pl) {
    assert(*it == num);
    ++it;
  }

  assert(it == l.end());

  for (auto it = pl->begin(); it != pl->end(); ++it) {
    assert(it == pl->lower_bound(it));
  }

  assert(pl->words() == l.size());

  free(pl);
}

int main() {
  srand(0);
  list<post<Text>> l;
  
  uint64_t base = 0;

  for (int i = 0; i < 1000; ++i) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  test(l);
  std::cout << "Pass test 1" << std::endl;

  while (l.size() < 20'000) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  test(l);
  std::cout << "Pass big test" << std::endl;
}
