#include "post_list.hpp"
#include "list.hpp"
#include "hashtable.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <ostream>

using namespace fast;

void test(const list<Offset> &l) {
  const auto space = post_list::size_needed(l);
  auto pl = (post_list*) malloc(space);

  post_list::write(l, pl);

  auto it = l.begin();
  for (const auto num : *pl) {
    assert(num == *it);
    assert(it != l.end());
    ++it;
  }
  assert(it == l.end());

  auto pit = pl->begin();
  pit.seek_forward(l.back());

  std::cout << *pit << std::endl;
  std::cout << l.back() << std::endl;
  assert(*pit == l.back());
  assert(pit);
  ++pit;
  assert(!pit);

  free(pl);
}
int main() {
  srand(0);
  list<Offset> l;

  Offset base = 0;

  while (l.size() < 100) {
    l.push_back(base);
    base += rand() % 100;
    ++base;
  }

  test(l);

  while (l.size() < 10000) {
    l.push_back(base);
    base += rand() % 100;
    ++base;
  }

  test(l);
}
