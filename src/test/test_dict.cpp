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
    const auto p = buffer->get(s);
    assert(p.second);
    assert(p.first == 1);
    buffer->put(s, 2);
  }

  for (const auto &s : strings) {
    assert(buffer->get(s).first == 2);
    buffer->put(s, 3);
    assert(buffer->get(s).first == 3);
  }

  assert(buffer->get("noin").second == false);
  free(buffer);
}
