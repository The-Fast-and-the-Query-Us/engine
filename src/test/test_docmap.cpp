#include "url_map.hpp"
#include "list.hpp"
#include "hashtable.hpp"
#include <cassert>
#include <cstdlib>

using namespace fast;

const uint64_t DONT = 69;

void test(const list<Url> &l) {
  const auto space = url_map::size_needed(l);
  auto um = (url_map*) malloc(space);
  url_map::write(l, um);

  for (const auto &u : l) {
    assert(um->get(u.second) == u.first);
  }

  assert(um->get(DONT) == "");
  free(um);
}

string urls[] = {"www.amazon", "www.google.com", "www.github.com", "www.umich.edu"};

int main() {
  srand(0);

  list<Url> l;

  for (int i = 0; i < 100; ++i) {
    l.emplace_back(urls[i % 4], rand());
  }

  test(l);
}
