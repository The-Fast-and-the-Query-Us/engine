#pragma once

#include <isr.hpp>
#include <string.hpp>

namespace fast::query {

/* 
BNF:

  Requirements
  1) words
  2) Phrases
  3) AND, OR, NOT
  4) Parenthesis

  Definitions

  <Constraint> ::= <BaseConstraint> { <OrOp> <BaseConstraint> }
  <OrOp> ::= 'OR'
  <BaseConstraint> ::= <SimpleConstaint> { [ <AndOp> ] <SimpleConstraint> }
  <AndOp> ::= 'AND'
  <SimpleConstraint> ::= <Phrase> | <NestedConstraint> | <UnaryOp> <SimpleConstraint> | <SearchWord>
  <UnaryOp> ::= 'NOT'
  <Phrase> ::= '"' { <SearchWord> } '"'
  <NestedConstraint> ::= '(' <Constraint> ')'
*/

class query_stream {
  const string_view query;
  size_t pos;

  public:

  query_stream(const string &query) : query(query), pos(0) {};

  bool match_or();
};

class contraint_parser {
  public:

  static isr *parse_contraint(query_stream &query) {
    auto bases = (isr**) malloc(sizeof(isr*));
    bases[0] = parse_base_contraint(query);

    if (!bases[0]) return nullptr;

  }
  static isr *parse_base_contraint(query_stream &query);
  static isr *parse_simple_contraint(query_stream &query);

};


}
