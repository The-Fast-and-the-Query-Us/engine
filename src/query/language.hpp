#pragma once

namespace fast::query {

/* BNF:

<Constraint>       ::= <BaseConstraint> { <OrOp> <BaseConstraint> }
<OrOp>             ::= 'OR' | '|' | '||'
<BaseConstraint>   ::= <SimpleConstaint> { [ <AndOp> ] <SimpleConstraint> }
<AndOp>            ::= 'AND' | '&' | '&&'
<SimpleConstraint> ::= <Phrase> | <NestedConstraint> | <UnaryOp> <SimpleConstraint> | <SearchWord>
<UnaryOp>          ::= '+' | '-' | 'NOT'
<Phrase>           ::= '"' { <SearchWord> } '"'
<NestedConstraint> ::= '(' <Constraint> ')'

*/

}
