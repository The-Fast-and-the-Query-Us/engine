#pragma once

#include <hashblob.hpp>
#include <isr.hpp>
#include <vector.hpp>

namespace fast::query {

constexpr size_t SHORT_SPAN_LENGTH = 10;

class flattened_query {
 public:
  flattened_query(const hashblob* index_chunk, const string& raw_query) {
    // flatten query into unique words
    unique_words = get_unique_words(raw_query);
    sz = unique_words.size();

    isrs = (isr**)malloc((sz - 1) * sizeof(isr*));

    // get rarest word
    string min_word;
    size_t min_num_words = ~0;
    for (auto& word : unique_words) {
      size_t num_words = index_chunk->get(word)->words();
      if (num_words < min_num_words) {
        min_word = word;
        min_num_words = num_words;
        // rarest is first in list
        swap(word, unique_words[0]);
      }
    }

    rarest_isr = new isr_word(unique_words[0]);

    for (size_t i = 1; i < sz; ++i) {
      isrs[i - 1] = new isr_word(unique_words[i]);
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
  vector<string> unique_words;
  size_t sz;

  isr* rarest_isr;
  isr** isrs;
};
}  // namespace fast::query
