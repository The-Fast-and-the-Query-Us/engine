#include <cstdlib>
#include <post_list.hpp>
#include <list.hpp>
#include <hashtable.hpp>

using namespace fast;

int main(void) {
  srand(0);
  list<Offset> l;

  Offset base = 0;

  while (l.size() < 100) {
    l.push_back(base);
    base += (rand() % 100) + 1;
  }

  const auto needed = post_list::size_needed(l);

  auto pl = (post_list*) malloc(needed);
  post_list::write(l, pl);

  free(pl);
}
