#include "list.hpp"
#include "postlist.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>

using namespace fast;

void test(const list<uint64_t> &l) {
  const auto space = postlist::size_needed(l);
  auto pl = (postlist*) malloc(space);
  memset(pl, 0, space);
  postlist::write(l, pl);

  auto it = l.begin();
  for (const auto num : *pl) {
    assert(*it == num);
    ++it;
  }

  assert(it == l.end());

  for (auto it = pl->begin(); it != pl->end();) {
    const auto num = *it;
    assert(pl->upper_bound(num) == ++it);
  }

  free(pl);
}

int main() {
  srand(0);
  list<uint64_t> l;
  
  uint64_t base = 0;

  for (int i = 0; i < 1000; ++i) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  test(l);


  while (l.size() < 20'000) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  test(l);
}
