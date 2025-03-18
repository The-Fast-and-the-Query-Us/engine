#include "list.hpp"
#include "postlist.hpp"
#include <cassert>
#include <cstdint>
#include <cstdlib>

using namespace fast;

int main() {
  srand(0);
  list<uint64_t> l;
  
  uint64_t base = 0;

  for (int i = 0; i < 1000; ++i) {
    l.push_back(base);
    base += (rand() % 10000) + 1;
  }

  const auto space = postlist::size_needed(l);

  auto pl = (postlist*) malloc(space);
  memset(pl, 0, space);

  postlist::write(l, pl);

  auto it = l.begin();

  for (const auto num : *pl) {
    assert(it != l.end());
    assert(*it == num);
    ++it;
  }

}
