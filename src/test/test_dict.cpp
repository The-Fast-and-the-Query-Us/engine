#include "common.hpp"
#include <cassert>
#include <cstdlib>
#include <cstring>
#include <hashtable.hpp>
#include <string.hpp>
#include <dictionary.hpp>

using namespace fast;
int main() {

  const string strings[] = {"a", "word", "", "12749hjsd", "</tag?>", "another", "yer"};

  hashtable ht;

  for (const auto &s : strings) ht.add(s);

  assert(ht.tokens() == sizeof(strings) / sizeof(string));

  const auto opts = dictionary::size_required(ht);

  auto buffer = (dictionary*) malloc(round_up(opts, alignof(dictionary)));
  memset(buffer, 0, opts);

  dictionary::write(ht, buffer);

  for (const auto &s : strings) {
    auto val = (*buffer)[s.c_str()];
    assert(*val);
    assert(val == 1);
    val = 2;
  }

  auto val = (*buffer)["notin"];
  assert(*val == false);

  free(buffer);
}
