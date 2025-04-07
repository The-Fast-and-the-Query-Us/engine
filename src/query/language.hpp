#pragma once

#include <cstdio>
#include <hashblob.hpp>
#include <isr.hpp>
#include <string.hpp>

#include "isr.hpp"

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

  example: wikipedia | wikihow + minecraft - programming + [ creeper | zombie ]
+ " steve jobs "
*/

class query_stream {
  const string query;
  size_t pos;

  bool important(char c) {
    switch (c) {
      case '+':
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
    return (pos < query.size() && c == query[pos]);
  }

  bool match(char c) {
    skip_ws();
    if (pos == query.size() || query[pos] != c) return false;
    ++pos;
    return true;
  }

  string_view get_word() {
    skip_ws();
    auto start = query.begin() + pos;
    while (pos < query.size() && !word_break(query[pos])) ++pos;
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
        ans->seek(0);  // init
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

  static isr *parse_simple_contraint(query_stream &query,
                                     const hashblob *blob) {
    if (query.match('[')) {
      if (query.match(']')) return new isr_null;  // isr any?

      auto ans = parse_contraint(query, blob);

      if (!query.match(']')) {
        delete ans;
        return nullptr;
      }

      return ans;

    } else if (query.match('"')) {
      if (query.match('"')) return new isr_null;  // maybe return isr_any here?

      auto ans = new isr_phrase;
      while (!query.match('"')) {
        auto word = query.get_word();
        if (query.is_end()) { // invalid query
          delete ans;
          return nullptr;
        }
        ans->add_stream(get_body_or_title(word, blob));
      }

      return ans;

    } else {
      auto word = query.get_word();
      return get_body_or_title(word, blob);
    }
  }

  static isr *get_body_or_title(string word, const hashblob *blob) {
    auto ans = new isr_or; 
    auto pl = blob->get(word);
    if (pl) {
      ans->add_stream(pl->get_isr());
    } else {
      ans->add_stream(new isr_null);
    }

    word += '#';
    pl = blob->get(word);

    if (pl) {
      ans->add_stream(pl->get_isr());
    } else {
      ans->add_stream(new isr_null);
    }

    return ans;
  }
};

class rank_parser {
 public:
  static void parse_query(query_stream &query, vector<string_view> *words) {
    parse_base_query(query, words);

    while (1) {
      if (query.match('+')) {
        parse_base_query(query, words);
      } else if (query.match('-')) {
        parse_base_query(query, nullptr);
      } else {
        return;
      }
    }
  }

  static void parse_base_query(query_stream &query, vector<string_view> *words) {
    parse_simple_query(query, words);

    while (query.match('|')) {
      parse_simple_query(query, words);
    }
  }

  static void parse_simple_query(query_stream &query, vector<string_view> *words) {
    if (query.match('[')) {
      parse_query(query, words);
      query.match(']');
    } else if (query.match('"')) {
      while (!query.match('"')) {
        if (query.is_end()) return;
        auto word = query.get_word();
        if (words) {
          words->push_back(word);
        };
      }
    } else {
      auto word = query.get_word();
      if (words) {
        words->push_back(word);
      }
    }
  }
};

}  // namespace fast::query
