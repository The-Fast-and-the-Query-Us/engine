#include <cassert>
#include <cstdlib>
#include <isr.hpp>
#include <post_list.hpp>
#include <../query/isr.hpp>

const int MAX = 10'000;

int main() {
  srand(0);
  fast::list<fast::Offset> phrases;

  fast::list<fast::Offset> lists[3];

  fast::Offset base = 0;
  for (auto i = 0; i < MAX; ++i) {
    if (rand() % 10 == 0) {
      phrases.push_back(base);
      lists[0].push_back(base++);
      lists[1].push_back(base++);
      lists[2].push_back(base++);
    } else {
      lists[2].push_back(base++);
      lists[1].push_back(base++);
      lists[0].push_back(base++);
    }
  }

  fast::post_list *pls[3];
  for (int i = 0; i < 3; ++i) {
    pls[i] = (fast::post_list*) malloc(fast::post_list::size_needed(lists[i]));
    fast::post_list::write(lists[i], pls[i]);
  }

  fast::query::isr_phrase phrase;
  for (int i = 0; i < 3; ++i) {
    phrase.add_stream(pls[i]->get_isr());
  }

  phrase.seek(0); // init
  for (const auto num : phrases) {
    assert(!phrase.is_end());
    assert(phrase.offset() == num);
    phrase.next();
  }

  assert(phrase.is_end());

  for (int i = 0; i < 3; ++i) {
    free(pls[i]);
  }
}
