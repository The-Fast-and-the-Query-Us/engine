#include "post_list.hpp"
#include "list.hpp"
#include "hashtable.hpp"
#include <cassert>
#include <cstdlib>

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

  free(pl);
}
int main() {
  srand(0);
  list<Offset> l;

  while (l.size() < 100) {
    l.push_back((rand() % 100) + 1);
  }

  test(l);

  while (l.size() < 10000) {
    l.push_back((rand() % 500) + 1);
  }

  test(l);
}
