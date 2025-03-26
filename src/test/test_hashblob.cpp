#include <cassert>
#include <cstdlib>
#include <hashblob.hpp>
#include <hashtable.hpp>

using namespace fast;

const string_view words[] = {"word", "skjdfbk", "sdajfkbsdfk", "cpoffeeafjosnwsfj", "worrds"};

const string_view urls[] = {"www.google.com", "github.com", "www.youtube.com"};

int main() {
  hashtable ht;

  for (const auto &url : urls) {
    for (const auto &word : words) {
      ht.add(word);
    }
    ht.doc_end(url);
  }

  const auto space = hashblob::size_required(ht);

  auto hb = (hashblob*) malloc(space);
  memset(hb, 0, space);
  hashblob::write(ht, hb);

  {
    auto it = hb->docs()->begin<Doc>();
    for (const auto &url : urls) {
      assert(url == it.url());
      assert(it != hb->docs()->end<Doc>());
      ++it;
    }
    assert(it == hb->docs()->end<Doc>());
  }

  uint64_t i = 0;
  for (const auto &word : words) {
    auto list = hb->get(word.begin());
    assert(list->words() == 3);
    auto j = i;
    for (auto it = list->begin<Text>(); it != list->end<Text>(); ++it) {
      assert(it == j);
      j += 6;
    }
    ++i;
  }

  free(hb);
}
