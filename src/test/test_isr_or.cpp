#include <cassert>
#include <cstdlib>
#include <hashtable.hpp>
#include <list.hpp>
#include <post_list.hpp>
#include <unistd.h>

// fix later
#include "../query/isr.hpp"

using namespace fast;

int main() {
  srand(0);

  list<Offset> ls[3];

  Offset base = 0;

  for (auto i = 0; i < 10'000; ++i) {
    ls[rand() % 2].push_back(base);
    ls[2].push_back(base);

    base += (rand() % 100) + 1;
  }

  post_list *pls[2];
  isr *isrs[2];

  for (int i = 0; i < 2; ++i) {
    pls[i] = (post_list*) malloc(post_list::size_needed(ls[i]));
    post_list::write(ls[i], pls[i]);
    isrs[i] = pls[i]->get_isr();
  }

  query::isr_or ors(isrs, 2);

  for (const auto num : ls[2]) {
    assert(!ors.is_end());
    assert(ors.offset() == num);
    ors.next();
  }

  assert(ors.is_end());


  for (int i = 0; i < 2; ++i) {
    delete isrs[i];
    free(pls[i]);
  }
}
