#pragma once

#include <array.hpp>
#include "constants.hpp"
#include <cstddef>
#include <hashblob.hpp>
#include <isr.hpp>
#include <string.hpp>
#include <vector.hpp>
#include "isr.hpp"
#include "language.hpp"

namespace fast::query {

void rank_from_constraints(const hashblob *blob, const string &query, 
                           array<Result, MAX_RESULTS> &results, isr_container *constraints) {
  vector<isr*> isrs;
  query_stream qs(query);
  rank_parser::parse_query(qs, blob, isrs);

  // TODO
}

constexpr size_t SHORT_SPAN_LENGTH = 10;

class ranker {
 public:
  ranker(const hashblob* index_chunk, const string& raw_query,
         isr_container* container) {
    // flatten query into unique words
    flattened = flatten_query(raw_query);
    sz = flattened.size();

    isrs = (isr**)malloc((sz) * sizeof(isr*));

    // get rarest word
    rare_word_idx = 0;
    size_t min_num_words = SIZE_MAX;
    for (size_t i = 0; i < sz; ++i) {
      size_t num_words = index_chunk->get(flattened[i])->words();
      if (num_words < min_num_words) {
        rare_word_idx = i;
        min_num_words = num_words;
      }
    }

    for (size_t i = 0; i < sz; ++i) {
      isrs[i] = index_chunk->get(flattened[i])->get_isr();
    }

    rarest_isr = isrs[rare_word_idx];

    // iterate through doc container
    while (!container->is_end()) {
      cur_doc_offset = container->get_doc_start();
      cur_doc_end = ;
      score_doc();

      container->next();
    }
  }

  void score_doc() {
    double score(0.0);

    score += count_full();

    if (sz > 2) {
      score += count_doubles();
    }

    if (sz > 3) {
      score += count_triples();
    }

    // static ranks

    // anchor text

    // title

    // url

    // insertion sort into top 10
    if (score < results[fast::query::MAX_RESULTS - 1].first) {
      return;
    }
    size_t i = fast::query::MAX_RESULTS - 1;

    results[i].first = score;
    results[i].second = "";
    while (i >= 1 && score > results[i - 1].first) {
      swap(results[i], results[i - 1]);
      --i;
    }
  }

  double count_spans(size_t num_isrs, isr** cur_isrs, size_t rare_idx) {
    vector<Offset> positions(num_isrs);
    vector<size_t> freqs(num_isrs);

    double score(0.0);
    for (size_t i = 0; i < num_isrs; ++i) {
      cur_isrs[i]->seek(cur_doc_offset);
      positions[i] = cur_isrs[i]->offset();
    }

    while (!cur_isrs[rare_idx]->is_end() &&
           cur_isrs[rare_idx]->offset() < cur_doc_end) {
      int span_length = get_span_length(num_isrs, cur_isrs, positions);
      if (span_length <= SHORT_SPAN_LENGTH) {
        // add to score
      }

      if (span_same_order(num_isrs, cur_isrs, positions)) {
        // add to score
      }

      if (span_phrase_match(num_isrs, cur_isrs, positions)) {
        // add to score
      }

      increment_isrs(num_isrs, cur_isrs, rare_idx, positions, freqs);
    }

    finish_counting(num_isrs, cur_isrs, positions, freqs);

    // count frequencies
    for (size_t i = 0; i < num_isrs; ++i) {
      // do something
    }
  }

  double count_full() {
    for (size_t i = 0; i < sz; ++i) {
      isrs[i]->seek(cur_doc_offset);
    }
    count_spans(sz, isrs, rare_word_idx);
  }

  double count_doubles() {
    isr* cur_isrs[2];
    for (size_t i = 0; i < sz; ++i) {
      if (i < rare_word_idx) {
        cur_isrs[0] = isrs[i];
        cur_isrs[1] = isrs[rare_word_idx];
        count_spans(2, cur_isrs, 1);
      } else if (i > rare_word_idx) {
        cur_isrs[0] = isrs[rare_word_idx];
        cur_isrs[1] = isrs[i];
        count_spans(2, cur_isrs, 0);
      }
    }
  }

  double count_triples() {
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

        count_spans(3, cur_isrs, rare_idx);
      }
    }
  }

  size_t size() { return sz; }

 private:
  size_t get_span_length(size_t num_isrs, isr** cur_isrs,
                         vector<Offset>& positions) {
    Offset min_off = rarest_isr->offset();
    Offset max_off = rarest_isr->offset() + 1;

    for (size_t i = 0; i < num_isrs; ++i) {
      min_off = min(min_off, cur_isrs[i]->offset());
      max_off = max(max_off, cur_isrs[i]->offset());
    }

    return max_off - min_off;
  }

  bool span_same_order(size_t num_isrs, isr** isrs,
                       const vector<Offset>& positions) {
    for (size_t i = 0; i < num_isrs - 1; ++i) {
      if (positions[i] >= positions[i + 1]) {
        return false;
      }
    }
    return true;
  }

  bool span_phrase_match(size_t num_isrs, isr** isrs,
                         const vector<Offset>& positions) {
    for (size_t i = 0; i < num_isrs - 1; ++i) {
      if (positions[i] + 1 != positions[i + 1]) {
        return false;
      }
    }
    return true;
  }

  void increment_isrs(size_t num_isrs, isr** isrs, size_t rare_idx,
                      vector<Offset>& positions, vector<size_t>& freqs) {
    isrs[rare_idx]->next();
    Offset target_pos = isrs[rare_idx]->offset();
    if (target_pos >= cur_doc_end) {
      return;
    }
    for (size_t i = 0; i < num_isrs; ++i) {
      if (i == rare_idx) {
        continue;
      }

      Offset cur_pos = isrs[i]->offset();

      while (abs(target_pos - cur_pos) < abs(target_pos - positions[i]) &&
             cur_pos < cur_doc_end) {
        positions[i] = cur_pos;
        isrs[i]->next();
        ++freqs[i];
      }
    }
  }

  void finish_counting(size_t num_isrs, isr** isrs, vector<Offset>& positions,
                       vector<size_t>& freqs) {
    for (size_t i = 0; i < num_isrs; ++i) {
      while (positions[i] < cur_doc_end) {
        positions[i] = isrs[i]->offset();
        isrs[i]->next();
        ++freqs[i];
      }
    }
  }

 private:
  fast::array<fast::query::Result, fast::query::MAX_RESULTS> results;
  vector<string> flattened;
  size_t sz;
  Offset cur_doc_offset;
  Offset cur_doc_end;

  size_t rare_word_idx;
  isr* rarest_isr;
  isr** isrs;
};
}  // namespace fast
