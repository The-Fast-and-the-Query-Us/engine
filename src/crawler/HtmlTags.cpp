#include <cstring>
#include <cassert>
#include "HtmlTags.h"

static char lower(char c) {
  return ('A' <= c && c <= 'Z') ? c + 32 : c;
}

// -1 => name < tag
// 0 => name == tag
// 1 => name > tag
static int cmp(const char *name, const char *end, size_t idx) {
  auto tp = TagsRecognized[idx].Tag;
  while (name != end) {
    if (lower(*name) < *tp) return -1;
    else if (lower(*name) > *tp) return 1;
    ++name;
    ++tp;
  }

  return (*tp) ? -1 : 0;
}
// name points to beginning of the possible HTML tag name.
// nameEnd points to one past last character.
// Comparison is case-insensitive.
// Use a binary search.
// If the name is found in the TagsRecognized table, return
// the corresponding action.
// If the name is not found, return OrdinaryText.

DesiredAction LookupPossibleTag( const char *name, const char *nameEnd ) {
  size_t l = 0;
  size_t r = NumberOfTags;
  while (l < r) {
    const auto m = (l + r) >> 1;
    const auto c = cmp(name, nameEnd, m);
    if (c < 0) r = m;
    else if (c > 0) l = m + 1;
    else return TagsRecognized[m].Action;
  }
  return DesiredAction::OrdinaryText;
}
