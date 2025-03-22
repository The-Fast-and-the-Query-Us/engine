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

  free(hb);
}
