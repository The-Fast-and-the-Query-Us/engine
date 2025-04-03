#pragma once

#include <language.hpp>
#include <string.hpp>
#include <vector.hpp>

namespace fast {
class flatten_query {
 public:
  static vector<string> *parse_contraint(query_stream &query,
                                         const hashblob *blob,
                                         vector<string> &flattened) {
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
}  // namespace fast