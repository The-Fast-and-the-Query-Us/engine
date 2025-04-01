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
  isr_doc *other = nullptr;
  for (const auto num : l) {
    assert(isr->next());
    assert(isr->offset() == num);

    if (other) {
      assert(other->offset() == isr->offset());
      delete other;
    }
    auto other_isr = pl->get_isr();
    other_isr->seek(isr->offset());
  }
  delete other;
  assert(!isr->next());
  delete isr;

  isr = pl->get_isr();
  assert(!isr->seek(l.back() + 1));
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
