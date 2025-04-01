#pragma once

#include <isr.hpp>
#include <string.hpp>
#include "isr.hpp"
#include <hashblob.hpp>

namespace fast::query {

/* 
BNF:

  Requirements
  1) words
  2) Phrases
  3) AND, OR, NOT
  4) Parenthesis

  Definitions

  <Constraint> ::= <BaseConstraint> { < + or - > <BaseConstraint> }
  <BaseConstraint> ::= <SimpleConstaint> { < | > <SimpleConstraint> }
  <SimpleConstraint> ::= <Phrase> | <NestedConstraint> | <SearchWord>
  <Phrase> ::= '"' { <SearchWord> } '"'
  <NestedConstraint> ::= '[' <Constraint> ']'

  example: wikipedia | wikihow + minecraft - programming
*/

class query_stream {
  const string_view query;
  size_t pos;

  bool important(char c) {
    switch (c) {
     case '+' :
     case '-':
     case '|':
     case '"':
     case '[':
     case ']':
      return true;
     default:
      return false;
    }
  }

  public:

  query_stream(const string &query) : query(query), pos(0) {};

  bool is_end() const { return pos == query.size(); }

  bool peek(char c) {
    return (
      pos < query.size() &&
      c == query[0]
    );
  }

  bool match(char c) {
    if (pos == query.size() || query[pos] != c) return false;
    ++pos;
    return true;
  }

  string_view get_word() {
    auto start = query.begin() + pos;
    while (pos < query.size() && !important(query[pos])) ++pos;
    return string_view(start, query.begin() + pos);
  }
};

class contraint_parser {
  public:

  static isr *parse_contraint(query_stream &query, const hashblob *blob) {
    auto left = parse_base_contraint(query, blob);

    if (!left) return nullptr;
    if (!query.peek('+') && !query.peek('-')) return left;

    auto ans = new isr_container(blob->docs()->get_doc_isr());
    ans->add_stream(left);

    while (1) {
      bool exclude;
      if (query.match('+')) {
        exclude = false;
      } else if (query.match('-')) {
        exclude = true;
      } else {
        ans->seek(0); // init
        return ans;
      }

      auto right = parse_base_contraint(query, blob);

      if (!right) {
        delete ans;
        return nullptr;
      }

      ans->add_stream(right, exclude);
    }
  }

  static isr *parse_base_contraint(query_stream &query, const hashblob *blob) {
    auto left = parse_simple_contraint(query, blob);

    if (!left) return nullptr;
    if (!query.peek('|')) return left;

    auto ans = new isr_or;
    ans->add_stream(left);

    while (query.match('|')) {
      auto right = parse_simple_contraint(query, blob);
      if (!right) {
        delete ans;
        return nullptr;
      }
      ans->add_stream(right);
    }

    return ans;
  }

  static isr *parse_simple_contraint(query_stream &query, const hashblob *blob) {
    if (query.match('[')) {
      // todo
    } else if (query.match('"')) {
      // todo
    } else {
      auto word = query.get_word();
      auto pl = blob->get(word);

      if (pl) {
        return pl->get_isr();
      } else {
        return new isr_null;
      }
    }
    return nullptr;
  }

};


}
