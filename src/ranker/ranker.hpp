#pragma once

#include <array.hpp>
#include <constants.hpp>
#include <hashblob.hpp>
#include <isr.hpp>
#include <string.hpp>
#include <vector.hpp>

namespace fast {
class ranker {
 public:
  ranker() {}

  void blob_rank(
      const fast::hashblob* index_chunk, const fast::string& raw_query,
      fast::array<fast::query::Result, fast::query::MAX_RESULTS>& results) {
    // preprocess
    fast::query::compiled_query compiled_query =
        fast::query::compile(raw_query);

    // constraint solver
    fast::vector<size_t> matching_docs;
    {
      fast::query::constraint_solver(compiled_query, index_chunk,
                                     matching_docs);
    }

    fast::query::flattened_query flattened_query =
        fast::query::flatten(raw_query);

    // score each document

    for (const size_t& doc_offset : matching_docs) {
      double score(0.0);

      // get rarest word
      fast::query::isr anchor_isr = flattened_query.rarest();

      flattened_query.reset_isrs();

      size_t spans = 0;
      size_t short_span_counts = 0;

      while (anchor_isr) {
        // increment other isrs to closest
        flattened_query.increment(*anchor_isr);
        size_t span_length = 0;  // span length

        if (/*valid span*/) {
          ++span_counts;
          span_length = tracker.span_length();
          if (span_length <= 10) {  // TODO: figure out thresholds
            ++short_span_counts;
          }
        }

        // match exact phrases

        // term frequencies
      }

      // span counts score

      // static ranks
      score += static_rank;

      // anchor text

      // title

      // url

      // insertion sort top k
    }
  }

 private:
};
}  // namespace fast