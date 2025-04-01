#pragma once

#include <array.hpp>
#include <constants.hpp>
#include <hashblob.hpp>
#include <isr.hpp>
#include <string.hpp>
#include <vector.hpp>

namespace fast {

constexpr size_t SHORT_SPAN_LENGTH = 10;

class ranker {
 public:
  static void blob_rank(
      const fast::hashblob* index_chunk, const fast::string& raw_query,
      const fast::list<size_t>& matching_docs,
      fast::array<fast::query::Result, fast::query::MAX_RESULTS>& results) {
    // preprocess
    fast::ranker ranker(index_chunk, raw_query);

    // score each document
    for (const size_t& doc_offset : matching_docs) {
      double score = ranker.score_doc(doc_offset);

      // insertion sort top k
      if (score < results[fast::query::MAX_RESULTS - 1].first) {
        continue;
      }
      size_t i = fast::query::MAX_RESULTS - 1;

      results[i].first = score;
      results[i].second = doc_offset;
      while (i >= 1 && score > results[i - 1].first) {
        swap(results[i], results[i - 1]);
        --i;
      }
    }
  }

  ranker(const hashblob* index_chunk, const string& raw_query) {
    // flatten query into unique words
    flattened = flatten_query(raw_query);
    sz = flattened.size();

    isrs = (isr**)malloc((sz - 1) * sizeof(isr*));

    // get rarest word
    string min_word;
    size_t min_num_words = ~0;
    for (auto& word : flattened) {
      size_t num_words = index_chunk->get(word)->words();
      if (num_words < min_num_words) {
        min_word = word;
        min_num_words = num_words;
        // rarest is first in list
        swap(word, flattened[0]);
      }
    }

    rarest_isr = new isr_word(flattened[0]);

    for (size_t i = 1; i < sz; ++i) {
      isrs[i - 1] = new isr_word(flattened[i]);
    }
  }

  double score_doc(size_t doc_offset) {
    double score(0.0);

    score += count_full(doc_offset);

    if (sz > 2) {
      score += count_doubles(doc_offset);
    }

    if (sz > 3) {
      score += count_triples(doc_offset);
    }

    // static ranks

    // anchor text

    // title

    // url
  }

  double count_spans(size_t doc_end, size_t num_isrs) {
    double score(0.0);
    while (/*rarest is less than doc_end*/) {
      // move other isrs to closest

      int span_length = get_span_length();
      if (span_length <= SHORT_SPAN_LENGTH) {  // TODO: figure out thresholds
        // add to score
      }
      // match exact phrases

      // term frequencies
    }
  }

  double count_full(size_t doc_offset) {
    rarest_isr->seek(doc_offset);

    for (size_t i = 0; i < sz - 1; ++i) {
      isrs[i]->seek(doc_offset);
    }
    count_spans(doc_offset, sz - 1);
  }

  double count_doubles(size_t doc_offset) {
    rarest_isr->seek(doc_offset);

    for (size_t i = 0; i < sz - 1; ++i) {
      swap(isrs[0], isrs[i]);
      isrs[0]->seek(doc_offset);
      count_spans(doc_offset, 1);
    }
  }

  double count_triples(size_t doc_offset) {
    rarest_isr->seek(doc_offset);

    for (size_t i = 0; i < sz - 1; ++i) {
      swap(isrs[0], isrs[i]);
      for (size_t j = i + 1; j < sz; ++j) {
        swap(isrs[1], isrs[j]);
        isrs[0]->seek(doc_offset);
        isrs[1]->seek(doc_offset);
        count_spans(doc_offset, 2);
      }
    }
  }

  size_t size() { return sz; }

 private:
  vector<string> flattened;
  size_t sz;

  isr* rarest_isr;
  isr** isrs;
};
}  // namespace fast