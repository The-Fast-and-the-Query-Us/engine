#pragma once

#include <cstdio>
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

  example: wikipedia | wikihow + minecraft - programming + [ creeper | zombie ] + " steve jobs "
*/

class query_stream {
  const string &query;
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

  // brainstorm
  bool word_break(char c) {
    switch (c) {
      case ' ':
      case '"':
      case '[':
      case ']':
        return true;
      default:
        return false;
    }
  }

  inline void skip_ws() {
    while (pos < query.size() && query[pos] == ' ') ++pos;
  }

  public:

  query_stream(const string &query) : query(query), pos(0) {};

  bool is_end() const { return pos == query.size(); }

  bool peek(char c) {
    skip_ws();
    return (
      pos < query.size() &&
      c == query[pos]
    );
  }

  // enforce ws?
  bool match(char c) {
    skip_ws();
    if (pos == query.size() || query[pos] != c) return false;
    ++pos;
    return true;
  }

  string_view get_word() {
    skip_ws();
    auto start = query.begin() + pos;
    while (pos < query.size() && query[pos] != ' ') ++pos;
    auto res = string_view(start, query.begin() + pos);
    return res;
  }
};

class contraint_parser {
  public:

  static isr_container *parse_contraint(query_stream &query, const hashblob *blob) {
    auto left = parse_base_contraint(query, blob);

    if (!left) return nullptr;

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
      if (query.match(']')) return new isr_null; // isr any?
      
      auto ans = parse_contraint(query, blob);

      if (!query.match(']')) {
        delete ans;
        return nullptr;
      }

      return ans;

    } else if (query.match('"')) {

      if (query.match('"')) return new isr_null; // maybe return isr_any here?

      auto ans = new isr_phrase;
      while (!query.match('"')) {
        auto word = query.get_word();
        if (query.is_end()) {
          delete ans;
          return nullptr;
        }

        auto pl = blob->get(word);

        if (pl) {
          ans->add_stream(pl->get_isr());
        } else {
          delete ans;
          return new isr_null;
        }
      }

      // maybe ans->seek(0) to init but container should call that
      return ans;

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
