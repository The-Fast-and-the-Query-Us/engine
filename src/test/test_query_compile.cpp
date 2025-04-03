#include <cassert>
#include <cstring>
#include <hashtable.hpp>
#include <hashblob.hpp>
#include "../query/language.hpp"

using namespace fast;

string_view abc = "abc", xyz = "xyz";

int main() {
  hashtable ht; 
  ht.add(abc);         // 0
  ht.add_doc("first");
  ht.add(xyz);         // 2
  ht.add_doc("second");
  ht.add(abc);         // 4
  ht.add(xyz);
  ht.add_doc("third");

  // phrase testing
  ht.add("123");       // 7
  ht.add("321");
  ht.add_doc("four");

  ht.add("321");       // 10
  ht.add("123");
  ht.add_doc("five");

  const auto space = hashblob::size_needed(ht);

  auto blob = (hashblob*) malloc(space);
  memset(blob, 0, space);
  hashblob::write(ht, blob);

  { // abc
    query::query_stream qs(abc);
    auto isr = query::contraint_parser::parse_contraint(qs, blob);
    assert(!isr->is_end());
    assert(isr->get_doc_start() == 0);
    isr->next();
    assert(!isr->is_end());
    assert(isr->get_doc_start() == 4);
    isr->next();
    assert(isr->is_end());
    delete isr;
  }

  { // abc
    query::query_stream qs(xyz);
    auto isr = query::contraint_parser::parse_contraint(qs, blob);
    assert(!isr->is_end());
    assert(isr->get_doc_start() == 2);
    isr->next();
    assert(!isr->is_end());
    assert(isr->get_doc_start() == 4);
    isr->next();
    assert(isr->is_end());
    delete isr;
  }

  { 
    query::query_stream qs("abc - xyz");
    auto isr = query::contraint_parser::parse_contraint(qs, blob);
    assert(!isr->is_end());
    assert(isr->get_doc_start() == 0);

    isr->next();
    assert(isr->is_end());
    delete isr;
  }

  {
    query::query_stream qs("abc + xyz");
    auto isr = query::contraint_parser::parse_contraint(qs, blob);
    assert(!isr->is_end());
    assert(isr->get_doc_start() == 4);

    isr->next();
    assert(isr->is_end());
    delete isr;
  }

  {
    query::query_stream qs("abc | xyz");
    auto isr = query::contraint_parser::parse_contraint(qs, blob);

    assert(!isr->is_end());
    assert(isr->get_doc_start() == 0);
    isr->next();

    assert(!isr->is_end());
    assert(isr->get_doc_start() == 2);
    isr->next();

    assert(!isr->is_end());
    assert(isr->get_doc_start() == 4);
    isr->next();

    assert(isr->is_end());
    delete isr;
  }

  {
    query::query_stream qs("\" 123 321 \"");
    auto isr = query::contraint_parser::parse_contraint(qs, blob);

    assert(!isr->is_end());
    assert(isr->get_doc_start() == 7);
    isr->next();
    assert(isr->is_end());

    delete isr;
  }


  free(blob);
}
