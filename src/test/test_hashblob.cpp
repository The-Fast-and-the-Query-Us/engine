#include "hashblob.hpp"
#include "hashtable.hpp"
#include "string.hpp"
#include <cassert>
#include <cstdlib>
#include <cstring>

using namespace fast;

const int NUM_WORDS = 6;
const int COUNT = 60'000;

static_assert(COUNT % NUM_WORDS == 0, "Must be even distribution");

string words[NUM_WORDS] = {"the", "abc", "rare", "abg", "james lu", "ayo"};


int main() {
  hashtable ht;

  for (auto i = 0; i < COUNT; ++i) {
    ht.add(words[i % NUM_WORDS]);
  }

  const auto space = hashblob::size_needed(ht);

  auto hb = (hashblob*) malloc(space);
  memset(hb, 0, space);
  hashblob::write(ht, hb);

  // count checks
  assert(hb->is_good());
  assert(hb->dict()->unique() == ht.unique());
  assert(hb->dict()->words() == ht.tokens());

  for (size_t i = 0; i < NUM_WORDS; ++i) {
    auto pl = hb->get(words[i]);

    assert(pl->words() == COUNT / NUM_WORDS);
    size_t acc = 0;
    for (const auto off : *pl) {
      assert(i + acc == off);
      acc += NUM_WORDS;
    }
  }

  free(hb);
}
