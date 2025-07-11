#include <cassert>
#include <cstdlib>
#include <post_list.hpp>
#include <list.hpp>
#include <hashtable.hpp>

using namespace fast;

void test(const list<Offset> &l) {
  const auto needed = post_list::size_needed(l);

  auto pl = (post_list*) malloc(needed);
  post_list::write(l, pl);

  assert(pl->words() == l.size());
  assert(pl->get_last() == l.back());

  // test iteration
  auto isr = pl->get_isr();
  for (const auto num : l) {
    assert(!isr->is_end());
    assert(isr->offset() == num);

    auto other_isr = pl->get_isr();
    other_isr->seek(isr->offset());
    assert(!other_isr->is_end());
    assert(other_isr->offset() == isr->offset());
    delete other_isr;

    isr->next();
  }

  assert(isr->is_end());
  delete isr;

  free(pl);
}

int main(void) {
  srand(0);
  list<Offset> l;

  Offset base = 0;

  while (l.size() < 100) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  test(l);

  while (l.size() < 10'000) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  test(l);
}
