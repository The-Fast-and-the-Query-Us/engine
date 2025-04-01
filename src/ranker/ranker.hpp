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
    rare_word_idx = 0;
    size_t min_num_words = ~0;
    for (size_t i = 0; i < sz; ++i) {
      size_t num_words = index_chunk->get(flattened[i])->words();
      if (num_words < min_num_words) {
        rare_word_idx = i;
        min_num_words = num_words;
      }
    }

    for (size_t i = 0; i < sz; ++i) {
      isrs[i] = new isr_word(flattened[i]);
    }

    rarest_isr = isrs[rare_word_idx];
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

  double count_spans(size_t doc_end, size_t num_isrs, isr** cur_isrs,
                     size_t rare_idx) {
    double score(0.0);
    while (/*rarest is less than doc_end*/) {
      // move other isrs to closest
      advance_isrs(doc_end, num_isrs, cur_isrs, rare_idx);

      int span_length = get_span_length(num_isrs, cur_isrs);
      if (span_length <= SHORT_SPAN_LENGTH) {
        // add to score
      }

      if (span_same_order()) {
        // add to score
      }

      if (span_phrase_match()) {
        // add to score
      }

      // term frequencies
    }
  }

  double count_full(size_t doc_offset) {
    for (size_t i = 0; i < sz; ++i) {
      isrs[i]->seek(doc_offset);
    }
    count_spans(doc_offset, sz, isrs, rare_word_idx);
  }

  double count_doubles(size_t doc_offset) {
    isr* cur_isrs[2];
    for (size_t i = 0; i < sz; ++i) {
      if (i < rare_word_idx) {
        cur_isrs[0] = isrs[i];
        cur_isrs[1] = isrs[rare_word_idx];
        cur_isrs[0]->seek(doc_offset);
        cur_isrs[1]->seek(doc_offset);
        count_spans(doc_offset, 2, cur_isrs, 1);
      } else if (i > rare_word_idx) {
        cur_isrs[0] = isrs[rare_word_idx];
        cur_isrs[1] = isrs[i];
        cur_isrs[0]->seek(doc_offset);
        cur_isrs[1]->seek(doc_offset);
        count_spans(doc_offset, 2, cur_isrs, 0);
      }
    }
  }

  double count_triples(size_t doc_offset) {
    rarest_isr->seek(doc_offset);
    isr* cur_isrs[3];
    size_t rare_idx = 3;
    for (size_t i = 0; i < sz - 1; ++i) {
      if (i == rare_word_idx) {
        continue;
      } else if (i < rare_word_idx) {
        cur_isrs[0] = isrs[i];
      } else {
        cur_isrs[0] = isrs[rare_word_idx];
        cur_isrs[1] = isrs[i];
        rare_idx = 0;
      }
      for (size_t j = i + 1; j < sz; ++j) {
        if (j == rare_word_idx) {
          continue;
        } else if (j < rare_word_idx) {
          cur_isrs[1] = isrs[j];
          cur_isrs[2] = isrs[rare_word_idx];
          rare_idx = 2;
        } else {
          if (rare_idx == 3) {
            cur_isrs[1] = isrs[rare_word_idx];
            cur_isrs[2] = isrs[j];
            rare_idx = 1;
          } else {
            cur_isrs[2] = isrs[j];
          }
        }
        cur_isrs[0]->seek(doc_offset);
        cur_isrs[1]->seek(doc_offset);
        cur_isrs[2]->seek(doc_offset);
        count_spans(doc_offset, 3, cur_isrs, rare_idx);
      }
    }
  }

  size_t size() { return sz; }

 private:
  void advance_isrs(size_t doc_end, size_t num_isrs, isr** cur_isrs) {
    for (size_t i = 0; i < num_isrs; ++i) {
      while (/*moving isrs[i] moves it closer to rarest_isr*/) {
        // update isr[i]
      }
    }
  }

  size_t get_span_length(size_t num_isrs, isr** cur_isrs) {
    Offset min_off = rarest_isr->offset();
    Offset max_off = rarest_isr->offset() + 1;

    for (size_t i = 0; i < num_isrs; ++i) {
      min_off = min(min_off, cur_isrs[i]->offset());
      max_off = max(max_off, cur_isrs[i]->offset());
    }

    return max_off - min_off;
  }

  bool span_same_order(isr** isrs) {}

  bool span_phrase_match(isr** isrs) {}

 private:
  vector<string> flattened;
  size_t sz;

  size_t rare_word_idx;
  isr* rarest_isr;
  isr** isrs;
};
}  // namespace fast