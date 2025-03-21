#include "list.hpp"
#include "postlist.hpp"
#include "types.hpp"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string_view>
#include "string_view.hpp"

using namespace fast;
using namespace fast::index;

const string_view urls[] = {"www.google.com", 
  "sdjkgbskjfhjksdfnksjdfjnskd something hella big", "a", 
  "medium", "www.nytimes.com"};

void test(list<post<Doc>> &l) {
  const auto space = postlist<Doc>::size_needed(l);
  auto pl = (postlist<Doc>*) malloc(space);

  postlist<Doc>::write(l, pl);

  auto is = pl->begin();
  for (auto it = l.begin(); it != l.end(); ++it) {
    const auto url = is.url();
    std::cout << "in list" << std::string_view((*it).url.begin(), (*it).url.size()) << std::endl;
    std::cout << "post url" << std::string_view(url.begin(), url.size()) << std::endl;

    assert(*it == is);
    assert(is.url() == urls[is % 5]);
    ++is;
  }

  assert(is == pl->end());

  for (auto it = pl->begin(); it != pl->end(); ++it) {
    assert(it == pl->lower_bound(it));
  }

  assert(pl->words() == l.size());
  
  free(pl);
}

int main() {
  srand(0);
  list<post<Doc>> l;
  uint64_t base = 0;

  while (l.size() < 10) {
    l.emplace_back(urls[base % 5], base);
    base += (rand() % 100) + 1;
  }

  test(l);

}
