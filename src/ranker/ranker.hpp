#pragma once

#include <array.hpp>
#include <constants.hpp>
#include <flattened_query.hpp>
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
      const fast::list<size_t>& matching_docs,
      fast::array<fast::query::Result, fast::query::MAX_RESULTS>& results) {
    // preprocess
    fast::query::flattened_query flattened_query(index_chunk, raw_query);

    // score each document
    for (const size_t& doc_offset : matching_docs) {
      double score = flattened_query.score_doc(doc_offset);

      // insertion sort top k
      if (score < results[9].first) {
        continue;
      }
      size_t i = 9;

      results[i].first = score;
      results[i].second = doc_offset;
      while (i >= 1 && score > results[i - 1].first) {
        swap(results[i], results[i - 1]);
        --i;
      }
    }
  }

 private:
};
}  // namespace fast