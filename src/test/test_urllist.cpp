#include <cassert>
#include <cstdlib>
#include <hashtable.hpp>
#include <list.hpp>
#include <post_list.hpp>

using namespace fast;

string urls[] = {"www.amazon.com", "www.google.com", "www.github.com", "s", "helllaskjfjhsndolfnowdefnwifbnsjk"};

const int NUM_URL = sizeof(urls) / sizeof(string);

int main() {
  list<url_post> l;
  
  Offset base = 0;
  while (l.size() < 100) {
    l.emplace_back(rand(), base, urls[base %NUM_URL]);
    base += (rand() % 100) + 1;
  }

  const auto space = post_list::size_needed(l);

  auto pl = (post_list*) malloc(space);
  post_list::write(l, pl);

  assert(pl->words() == l.size());
  assert(pl->get_last() == l.back().offset);

  free(pl);
}
