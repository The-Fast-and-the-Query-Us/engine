#pragma once

#include <array.hpp>
#include <cstddef>
#include <hashblob.hpp>
#include <isr.hpp>
#include <map>
#include <string.hpp>
#include <vector.hpp>

#include "constants.hpp"
#include "isr.hpp"
#include "language.hpp"
#include "url_components.hpp"

namespace fast::query {

constexpr size_t SHORT_SPAN_LENGTH = 10;

class ranker {
 public:
  ranker(const hashblob* index_chunk, vector<string_view>& sv,
         isr_container* container, array<Result, MAX_RESULTS>& res)
      : results(res), flattened(sv) {
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

    // rarest_isr = isrs[rare_word_idx];

    // iterate through doc container
    while (!container->is_end()) {
      cur_doc_offset = container->get_doc_start();
      cur_doc_end = container->get_doc_end();
      cur_doc_url = container->get_doc_url();
      score_doc();

      container->next();
    }
  }

  ~ranker() {
    for (size_t i = 0; i < sz; ++i) {
      free(isrs[i]);
    };
    free(isrs);
  }

  void score_doc() {
    double score(0.0);

    if (sz == 1) {
      score += count_single();
    } else {
      score += count_full();

      if (sz > 2) {
        score += count_doubles();
      }

      if (sz > 3) {
        score += count_triples();
      }
    }

    // static ranks

    // title

    // url
    // score *= url_score();

    // insertion sort into top 10
    if (score < results[fast::query::MAX_RESULTS - 1].first) {
      return;
    }
    size_t i = fast::query::MAX_RESULTS - 1;

    results[i].first = score;
    results[i].second = cur_doc_url;
    while (i >= 1 && score > results[i - 1].first) {
      swap(results[i], results[i - 1]);
      --i;
    }
  }

  double count_single() {
    double score(0.0);
    isrs[0]->seek(cur_doc_offset);
    size_t count = 0;
    while (!isrs[0]->is_end() && isrs[0]->offset() < cur_doc_end) {
      ++count;
      isrs[0]->next();
    }
    score += 1000 * ((double)count) / ((double)cur_doc_end - cur_doc_offset);

    return score;
  }

  double count_spans(size_t num_isrs, isr** cur_isrs, size_t rare_idx,
                     std::map<string, vector<size_t>>* word_to_isrs = nullptr) {
    vector<Offset> positions(num_isrs);
    vector<size_t> freqs(num_isrs);  // frequency of words in doc

    double score(0.0);
    for (size_t i = 0; i < num_isrs; ++i) {
      cur_isrs[i]->seek(cur_doc_offset);
    }

    if (word_to_isrs) {
      for (auto& [word, word_isrs] : *word_to_isrs) {
        for (size_t i = 1; i < word_isrs.size(); ++i) {
          for (size_t j = 0; j < i; ++j) {
            cur_isrs[word_isrs[j]]->next();
            ++freqs[word_isrs[j]];
          }
        }
      }
    }

    for (size_t i = 0; i < num_isrs; ++i) {
      positions[i] = cur_isrs[i]->offset();
    }

    while (!cur_isrs[rare_idx]->is_end() &&
           cur_isrs[rare_idx]->offset() < cur_doc_end) {
      int span_length = get_span_length(num_isrs, rare_idx, positions);

      // add to score
      score += num_isrs * (25.0 / span_length);

      if (span_same_order(num_isrs, positions)) {
        // add to score
        score += num_isrs * 10.0;
      }

      if (span_phrase_match(num_isrs, positions)) {
        score += num_isrs * 25.0;
      }

      if (!word_to_isrs) {
        increment_isrs(num_isrs, cur_isrs, rare_idx, positions);
      } else {
        increment_isrs(num_isrs, cur_isrs, rare_idx, positions, freqs,
                       word_to_isrs);
      }
    }

    if (word_to_isrs) {
      finish_counting(num_isrs, cur_isrs, positions, freqs);

      size_t total_counts = 0;
      // count frequencies
      for (size_t i = 0; i < num_isrs; ++i) {
        // do something
        total_counts += freqs[i];
      }
      score += 1000 * ((double)total_counts) /
               ((double)cur_doc_end - cur_doc_offset);
    }

    return score;
  }

  double count_full() {
    std::map<string, vector<size_t>> word_to_isrs;
    for (size_t i = 0; i < sz; ++i) {
      word_to_isrs[flattened[i]].push_back(i);
    }
    return count_spans(sz, isrs, rare_word_idx, &word_to_isrs);
  }

  double count_doubles() {
    double score(0);
    isr* cur_isrs[2];
    for (size_t i = 0; i < sz; ++i) {
      if (flattened[i] == flattened[rare_word_idx]) {
        continue;
      } else if (i < rare_word_idx) {
        cur_isrs[0] = isrs[i];
        cur_isrs[1] = isrs[rare_word_idx];
      } else {
        cur_isrs[0] = isrs[rare_word_idx];
        cur_isrs[1] = isrs[i];
      }
      score += count_spans(2, cur_isrs, 1);
    }

    return score;
  }

  double count_triples() {
    double score(0);
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

        score += count_spans(3, cur_isrs, rare_idx);
      }
    }

    return score;
  }

  size_t size() { return sz; }

 private:
  size_t get_span_length(size_t num_isrs, size_t rare_idx,
                         vector<Offset>& positions) {
    Offset min_off = positions[rare_idx];
    Offset max_off = positions[rare_idx];

    for (size_t i = 0; i < num_isrs; ++i) {
      min_off = min(min_off, positions[i]);
      max_off = max(max_off, positions[i]);
    }

    return max_off - min_off;
  }

  bool span_same_order(size_t num_isrs, const vector<Offset>& positions) {
    for (size_t i = 0; i < num_isrs - 1; ++i) {
      if (positions[i] >= positions[i + 1]) {
        return false;
      }
    }
    return true;
  }

  bool span_phrase_match(size_t num_isrs, const vector<Offset>& positions) {
    for (size_t i = 0; i < num_isrs - 1; ++i) {
      if (positions[i] + 1 != positions[i + 1]) {
        return false;
      }
    }
    return true;
  }

  void increment_isrs(size_t num_isrs, isr** cur_isrs, size_t rare_idx,
                      vector<Offset>& positions, vector<size_t>& freqs,
                      std::map<string, vector<size_t>>* word_to_isrs) {
    cur_isrs[rare_idx]->next();
    Offset target_pos = cur_isrs[rare_idx]->offset();
    if (target_pos >= cur_doc_end) {
      return;
    }
    for (auto& [word, isr_pos] : *word_to_isrs) {
      if (word == flattened[rare_idx]) {
        continue;
      }
      Offset front = cur_isrs[isr_pos[0]]->offset();
      Offset back = cur_isrs[isr_pos.back()]->offset();

      Offset cur_span_length = max(target_pos - front, back - target_pos);

      if (target_pos < back && target_pos > front) {
        cur_span_length = back - front;
      }

      Offset prev_span_length = UINT32_MAX;

      while (cur_span_length < prev_span_length) {
        for (auto i : isr_pos) {
          positions[i] = cur_isrs[i]->offset();
          cur_isrs[i]->next();
          ++freqs[i];
        }
        front = cur_isrs[isr_pos[0]]->offset();
        back = cur_isrs[isr_pos.back()]->offset();
        prev_span_length = cur_span_length;
        cur_span_length = max(target_pos - front, back - target_pos);

        if (target_pos < back && target_pos > front) {
          cur_span_length = back - front;
        }
      }
    }
  }

  void increment_isrs(size_t num_isrs, isr** cur_isrs, size_t rare_idx,
                      vector<Offset>& positions) {
    cur_isrs[rare_idx]->next();
    Offset target_pos = cur_isrs[rare_idx]->offset();
    if (target_pos >= cur_doc_end) {
      return;
    }
    for (size_t i = 0; i < num_isrs; ++i) {
      if (i == rare_idx) {
        continue;
      }

      Offset cur_pos = cur_isrs[i]->offset();

      while (abs((int64_t)(target_pos) - (int64_t)(cur_pos)) <
                 abs((int64_t)(target_pos) - (int64_t)(positions[i])) &&
             cur_pos < cur_doc_end) {
        positions[i] = cur_pos;
        cur_isrs[i]->next();
        cur_pos = cur_isrs[i]->offset();
      }
    }
  }

  void finish_counting(size_t num_isrs, isr** cur_isrs,
                       vector<Offset>& positions, vector<size_t>& freqs) {
    for (size_t i = 0; i < num_isrs; ++i) {
      while (positions[i] < cur_doc_end) {
        positions[i] = cur_isrs[i]->offset();
        cur_isrs[i]->next();
        ++freqs[i];
      }
    }
  }

  double url_score() {
    double score(0.0);
    auto url_comp = url_components(cur_doc_url);

    return url_comp.get_multiplier();
  }

 private:
  fast::array<fast::query::Result, fast::query::MAX_RESULTS>& results;

  vector<string_view>& flattened;
  size_t sz;

  Offset cur_doc_offset;
  Offset cur_doc_end;
  string_view cur_doc_url;

  size_t rare_word_idx;
  // isr* rarest_isr;
  isr** isrs;
};

void blob_rank(
    const fast::hashblob* blob, const fast::string& query,
    fast::array<fast::query::Result, fast::query::MAX_RESULTS>& results) {
  auto query_stream = fast::query::query_stream(query);
  auto constraints =
      fast::query::contraint_parser::parse_contraint(query_stream, blob);

  if (!constraints) return;

  fast::vector<fast::string_view> flattened;

  auto rank_stream = fast::query::query_stream(query);
  fast::query::rank_parser::parse_query(rank_stream, &flattened, blob);
  fast::query::ranker(blob, flattened, constraints, results);

  delete constraints;
  return;
}

}  // namespace fast::query
