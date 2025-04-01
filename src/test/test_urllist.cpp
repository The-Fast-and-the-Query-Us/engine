#include <cassert>
#include <cstdlib>
#include <hashtable.hpp>
#include <list.hpp>
#include <post_list.hpp>

using namespace fast;

string urls[] = {"www.amazon.com", "www.google.com", "www.github.com", "s", "helllaskjfjhsndolfnowdefnwifbnsjk"};

const int NUM_URL = sizeof(urls) / sizeof(string);

Offset base = 0;
list<url_post> l;

void generate(size_t len) {
  while (l.size() < len) {
    l.emplace_back(rand(), base, urls[base %NUM_URL]);
    base += (rand() % 100) + 1;
  }
}

void test(void) {
  const auto space = post_list::size_needed(l);

  auto pl = (post_list*) malloc(space);
  post_list::write(l, pl);

  assert(pl->words() == l.size());
  assert(pl->get_last() == l.back().offset);

  auto isr = pl->get_doc_isr();
  for (const auto &post : l) {
    assert(!isr->is_end());
    assert(isr->offset() == post.offset);
    assert(isr->len() == post.doc_len);
    assert(isr->get_url() == urls[isr->offset() % NUM_URL]);

    auto other = pl->get_doc_isr();
    other->seek(isr->offset());
    
    assert(!other->is_end());
    assert(other->offset() == isr->offset());
    assert(other->get_url() == isr->get_url());
    assert(other->len() == isr->len());

    delete other;

    isr->next();
  }

  assert(isr->is_end());
  delete isr;

  free(pl);
}

int main() {
  generate(100);
  test();
  generate(10'000);
  test();
}
