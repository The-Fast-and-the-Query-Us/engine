#pragma once

#include <array.hpp>
#include <cctype>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <functional>
#include <hashblob.hpp>
#include <isr.hpp>
#include <map>
#include <string.hpp>
#include <sys/fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector.hpp>
#include <sys/mman.h>

#include "common.hpp"
#include "constants.hpp"
#include "isr.hpp"
#include "language.hpp"
#include "url_components.hpp"

namespace fast::query {

constexpr size_t SHORT_SPAN_LENGTH = 10;
constexpr double BASE_PROXIMITY = 25.0;
constexpr double DECAY_RATE = 1.5;
constexpr double POSITION_BOOST_MAX = 2.0;
constexpr double POSITION_BOOST_MIN = 0.6;

enum class meta_stream { TITLE, BODY };

std::map<meta_stream, double> meta_stream_mult = {{meta_stream::TITLE, 1.5},
                                                  {meta_stream::BODY, 1}};

class ranker {
 public:
  ranker(const hashblob* index_chunk, vector<string_view>& sv,
         isr_container* container, array<Result, MAX_RESULTS>& res)
      : results(res), flattened(sv) {
    sz = flattened.size();

    isrs = (isr**)malloc((sz) * sizeof(isr*));
    title_isrs = (isr**)malloc((sz) * sizeof(isr*));

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

    // intialize isrs for body
    for (size_t i = 0; i < sz; ++i) {
      auto* post_list = index_chunk->get(flattened[i]);
      isrs[i] = index_chunk->get(flattened[i])->get_isr();
    }

    title_isrs_sz = 0;
    min_num_words = SIZE_MAX;
    // initialize isrs for title
    for (size_t i = 0; i < sz; ++i) {
      string title_string(flattened[i].begin(), flattened[i].size());
      title_string += "#";
      auto* post_list = index_chunk->get(title_string);
      if (post_list != nullptr) {
        size_t num_words = post_list->words();
        if (num_words < min_num_words) {
          rare_title_word_idx = title_isrs_sz;
          min_num_words = num_words;
        }
        title_isrs[title_isrs_sz++] = post_list->get_isr();
      }
    }

    // rarest_isr = isrs[rare_word_idx];

    // iterate through doc container
    while (!container->is_end()) {
      cur_doc_offset = container->get_doc_start();
      cur_doc_end = container->get_doc_end();
      cur_doc_url = container->get_doc_url();
      cur_doc_length = cur_doc_end - cur_doc_offset;
      score_doc();

      container->next();
    }
  }

  ~ranker() {
    for (size_t i = 0; i < sz; ++i) {
      free(isrs[i]);
    };
    free(isrs);

    for (auto i = 0; i < title_isrs_sz; ++i) {
      free(title_isrs[i]);
    }
    free(title_isrs);
  }

  void score_doc() {
    cur_doc_score = 0;

    // loop for all metastreams
    for (auto& [metastream, multiplier] : meta_stream_mult) {
      size_t cur_sz = metastream == meta_stream::BODY ? sz : title_isrs_sz;
      if (cur_sz == 0) {
        continue;
      }
      if (cur_sz == 1) {
        // do not look for spans
        cur_doc_score += multiplier * count_single(metastream);
      } else {
        // look for complete spans
        cur_doc_score += multiplier * count_full(metastream);

        // loop for spans of size 2
        if (cur_sz > 2) {
          cur_doc_score += multiplier * count_doubles(metastream);
        }

        // look for spans for size 3
        if (cur_sz > 3) {
          cur_doc_score += multiplier * count_triples(metastream);
        }
      }
    }

    // static ranks

    // title

    // url
    cur_doc_score *= url_score();

    // insertion sort into top 10
    if (cur_doc_score < results[fast::query::MAX_RESULTS - 1].first) {
      return;
    }
    size_t i = fast::query::MAX_RESULTS - 1;

    results[i].first = cur_doc_score;
    results[i].second = cur_doc_url;
    while (i >= 1 && cur_doc_score > results[i - 1].first) {
      swap(results[i], results[i - 1]);
      --i;
    }
  }

  double count_single(meta_stream ms) {
    // only one word queries
    if (ms == meta_stream::TITLE && title_isrs_sz == 0) {
      return 0;
    }
    isr** use_isrs = (ms == meta_stream::BODY) ? isrs : title_isrs;
    double score(0.0);
    vector<Offset> positions(1);
    vector<size_t> freqs(1);
    use_isrs[0]->seek(cur_doc_offset);

    isr* cur_isrs[1];
    cur_isrs[0] = use_isrs[0];
    positions[0] = use_isrs[0]->offset();

    score += count_single_scores(1, cur_isrs, positions, freqs);

    return score;
  }

  double count_spans(size_t num_isrs, isr** cur_isrs, size_t rare_idx,
                     std::map<string, vector<size_t>>* word_to_isrs = nullptr) {
    // current position of each isr
    vector<Offset> positions(num_isrs);
    // frequency of words in doc
    vector<size_t> freqs(num_isrs);

    double score(0.0);

    // seek all to front
    for (size_t i = 0; i < num_isrs; ++i) {
      cur_isrs[i]->seek(cur_doc_offset);
    }

    // if duplicate words, seek each into continuous span
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

    // update positions
    for (size_t i = 0; i < num_isrs; ++i) {
      positions[i] = cur_isrs[i]->offset();
    }

    while (all_in_doc(positions)) {
      double normalized_pos = (positions[0] - cur_doc_offset) / cur_doc_length;
      double pos_mult =
          POSITION_BOOST_MAX -
          (normalized_pos * (POSITION_BOOST_MAX - POSITION_BOOST_MIN));

      size_t span_length = get_span_length(num_isrs, rare_idx, positions);
      float proximity_score = BASE_PROXIMITY / log(DECAY_RATE + span_length);
      score += num_isrs * proximity_score * pos_mult;

      if (span_same_order(num_isrs, positions)) {
        // add to score
        score += num_isrs * 10.0 * pos_mult;
      }

      if (span_phrase_match(num_isrs, positions)) {
        score += num_isrs * 25.0 * pos_mult;
      }

      if (!word_to_isrs) {
        increment_isrs(num_isrs, cur_isrs, rare_idx, positions);
      } else {
        increment_isrs(num_isrs, cur_isrs, rare_idx, positions, freqs,
                       *word_to_isrs);
      }
    }

    // finish counting frequencies only once
    if (word_to_isrs) {
      score += count_single_scores(num_isrs, cur_isrs, positions, freqs);
    }

    return score;
  }

  double count_full(meta_stream ms) {
    // count complete query spans
    isr** cur_isrs = (ms == meta_stream::BODY) ? isrs : title_isrs;
    size_t cur_sz = (ms == meta_stream::BODY) ? sz : title_isrs_sz;
    size_t cur_rare_word_idx =
        (ms == meta_stream::BODY) ? rare_word_idx : rare_title_word_idx;
    std::map<string, vector<size_t>> word_to_isrs;
    for (size_t i = 0; i < cur_sz; ++i) {
      word_to_isrs[flattened[i]].push_back(i);
    }
    return count_spans(sz, cur_isrs, cur_rare_word_idx, &word_to_isrs);
  }

  double count_doubles(meta_stream ms) {
    // only two words of query
    isr** use_isrs = (ms == meta_stream::BODY) ? isrs : title_isrs;
    size_t cur_sz = (ms == meta_stream::BODY) ? sz : title_isrs_sz;
    size_t cur_rare_word_idx =
        (ms == meta_stream::BODY) ? rare_word_idx : rare_title_word_idx;

    double score(0);
    isr* cur_isrs[2];
    for (size_t i = 0; i < cur_sz; ++i) {
      if (flattened[i] == flattened[cur_rare_word_idx]) {
        continue;
      } else if (i < cur_rare_word_idx) {
        cur_isrs[0] = use_isrs[i];
        cur_isrs[1] = use_isrs[cur_rare_word_idx];
      } else {
        cur_isrs[0] = use_isrs[cur_rare_word_idx];
        cur_isrs[1] = use_isrs[i];
      }
      score += count_spans(2, cur_isrs, 1);
    }

    return score;
  }

  double count_triples(meta_stream ms) {
    // three word spans
    isr** use_isrs = (ms == meta_stream::BODY) ? isrs : title_isrs;
    size_t cur_sz = (ms == meta_stream::BODY) ? sz : title_isrs_sz;
    size_t cur_rare_word_idx =
        (ms == meta_stream::BODY) ? rare_word_idx : rare_title_word_idx;

    double score(0);
    isr* cur_isrs[3];
    size_t rare_idx = 3;
    for (size_t i = 0; i < cur_sz - 1; ++i) {
      if (i == cur_rare_word_idx) {
        continue;
      } else if (i < cur_rare_word_idx) {
        cur_isrs[0] = use_isrs[i];
      } else {
        cur_isrs[0] = use_isrs[cur_rare_word_idx];
        cur_isrs[1] = use_isrs[i];
        rare_idx = 0;
      }
      for (size_t j = i + 1; j < cur_sz; ++j) {
        if (j == cur_rare_word_idx) {
          continue;
        } else if (j < cur_rare_word_idx) {
          cur_isrs[1] = use_isrs[j];
          cur_isrs[2] = use_isrs[cur_rare_word_idx];
          rare_idx = 2;
        } else {
          if (rare_idx == 3) {
            cur_isrs[1] = use_isrs[cur_rare_word_idx];
            cur_isrs[2] = use_isrs[j];
            rare_idx = 1;
          } else {
            cur_isrs[2] = use_isrs[j];
          }
        }

        score += count_spans(3, cur_isrs, rare_idx);
      }
    }

    return score;
  }

  size_t size() { return sz; }

 private:
  bool all_in_doc(vector<Offset>& positions) {
    for (size_t i = 0; i < positions.size(); ++i) {
      if (positions[i] >= cur_doc_end) {
        return false;
      }
    }
    return true;
  }

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
                      std::map<string, vector<size_t>>& word_to_isrs) {
    // increment rare group
    for (auto& isr_pos : word_to_isrs[flattened[rare_idx]]) {
      positions[isr_pos] = cur_isrs[isr_pos]->offset();
      cur_isrs[isr_pos]->next();
      ++freqs[isr_pos];
    }

    // rare_idx front and back
    Offset span_front =
        isrs[word_to_isrs[flattened[rare_idx]].front()]->offset();
    Offset span_back = isrs[word_to_isrs[flattened[rare_idx]].back()]->offset();

    if (span_back >= cur_doc_end) {  // what does is_end return?
      return;
    }

    // increment each word group
    for (auto& [word, isr_pos] : word_to_isrs) {
      if (word == flattened[rare_idx]) {
        continue;
      }

      // calculate current span length
      Offset front = cur_isrs[isr_pos.front()]->offset();
      Offset back = cur_isrs[isr_pos.back()]->offset();

      Offset cur_span_length = max(back, span_back) - min(front, span_front);

      Offset prev_span_length = UINT32_MAX;

      // while span can get smaller
      while (cur_span_length < prev_span_length) {
        // increment all of group
        for (auto i : isr_pos) {
          double normalized_pos =
              (positions[i] - cur_doc_offset) / cur_doc_length;
          double pos_mult =
              POSITION_BOOST_MAX -
              (normalized_pos * (POSITION_BOOST_MAX - POSITION_BOOST_MIN));

          cur_doc_score += 2 * pos_mult;

          positions[i] = cur_isrs[i]->offset();
          cur_isrs[i]->next();
          ++freqs[i];
        }

        // new span length
        front = cur_isrs[isr_pos.front()]->offset();
        back = cur_isrs[isr_pos.back()]->offset();
        if (back >= cur_doc_end) {
          break;
        }
        prev_span_length = cur_span_length;
        cur_span_length = max(back, span_back) - min(front, span_front);
      }
    }
  }

  void increment_isrs(size_t num_isrs, isr** cur_isrs, size_t rare_idx,
                      vector<Offset>& positions) {
    // increment rare
    cur_isrs[rare_idx]->next();
    Offset target_pos = cur_isrs[rare_idx]->offset();
    if (target_pos >= cur_doc_end) {
      return;
    }

    // increment each isr to closest pos
    for (size_t i = 0; i < num_isrs; ++i) {
      if (i == rare_idx) {
        continue;
      }

      Offset cur_pos = cur_isrs[i]->offset();

      // while not closer, increment
      while (abs((int64_t)(target_pos) - (int64_t)(cur_pos)) <
                 abs((int64_t)(target_pos) - (int64_t)(positions[i])) &&
             cur_pos < cur_doc_end) {
        positions[i] = cur_pos;
        cur_isrs[i]->next();
        cur_pos = cur_isrs[i]->offset();
      }
    }
  }

  double count_single_scores(size_t num_isrs, isr** cur_isrs,
                             vector<Offset>& positions, vector<size_t>& freqs) {
    double score(0.0);
    // finish counting all words
    for (size_t i = 0; i < num_isrs; ++i) {
      while (positions[i] < cur_doc_end) {
        double normalized_pos =
            (positions[i] - cur_doc_offset) / cur_doc_length;
        double pos_mult =
            POSITION_BOOST_MAX -
            (normalized_pos * (POSITION_BOOST_MAX - POSITION_BOOST_MIN));
        score += 2 * pos_mult;
        positions[i] = cur_isrs[i]->offset();
        cur_isrs[i]->next();
        ++freqs[i];
      }
    }
  }

  double url_score() {
    // static url score
    double score(0.0);
    auto url_comp = url_components(cur_doc_url);

    return url_comp.get_multiplier();
  }

 private:
  fast::array<fast::query::Result, fast::query::MAX_RESULTS>& results;

  vector<string_view>& flattened;
  size_t sz;
  size_t title_isrs_sz = 0;

  double cur_doc_score;

  Offset cur_doc_offset;
  Offset cur_doc_end;
  size_t cur_doc_length;
  string_view cur_doc_url;

  size_t rare_word_idx;
  size_t rare_title_word_idx;
  // isr* rarest_isr;
  isr** isrs;
  isr** title_isrs;
};

// tuning parameters
namespace Params {
static constexpr double 

Mtitle = 30,
ODouble = 1.0,
Double = 0.5,
OTriple = 1.0,
Triple = 0.5,
UrlDepth = -1,
Decay = 1.05,
SpanMult = 1.0,
DomainHit = 45.0,
AnyHit = 40.0,
UrlLength = -3,
GoodLength = 0.0,
GoodTLD = 75.0,
WhiteList = 30.0,
Ordered = 7.5,
Query = -40.0
;

static bool is_good_doc_len(Offset len) {
  return len >= 350 && len <= 2000;
}

};
/*
 * Insert result in decreasing order of score
 */
void insertion_sort(array<Result, MAX_RESULTS> &results, const Result &r) {
  if (results[MAX_RESULTS - 1].first < r.first) {
    results[MAX_RESULTS - 1] = r;

    for (size_t i = MAX_RESULTS - 1; i > 0; --i) {
      if (results[i].first > results[i - 1].first) {
        swap(results[i], results[i - 1]);
      } else {
        break;
      }
    }
  }
}

static double url_rank(const string_view &url, const vector<string_view> &words, size_t rare) {
  fast::string url_(url);

  for (auto &c : url_) c = tolower(c);

  if (url_.contains("updatestar") || url_.ends_with(".jpg") ||
      url_.ends_with(".png") || url_.ends_with(".svg") ||
      url_.ends_with("jpeg")) {

    return -1e20;
  }

  double score = 0;

  size_t slash_cnt = 0;
  for (const auto c : url_) {
    slash_cnt += c == '/';
  }

  if (url_.contains("en.wikipedia.org") || url_.contains("nytimes.com") ||
      url_.contains("cnn.com") || url_.contains("bbc.com") ||
      url_.contains("github.com")) {
    score += Params::WhiteList;
  }

  if (url_.contains(".com") || url_.contains(".edu") || url_.contains(".gov") 
    || url_.contains(".org")) {
    score += Params::GoodTLD;
  }

  if (url_.contains("?")) {
    score += Params::Query;
  }

  if (slash_cnt > 2) {
    score += Params::UrlDepth * (slash_cnt - 2);
  }

  score += Params::UrlLength * fast::fast_sqrt(url_.size());

  size_t cnt = 0;
  size_t ordered_ness = 0;
  fast::vector<size_t> pos(words.size());

  for (size_t i = 0; i < words.size(); ++i) {
    const auto m = url_.find(words[i]);

    if (m >= 0) {
      ++cnt;
      pos[i] = m;

      size_t order = 1;
      for (size_t j = 0; j < i; ++j) {
        order += (pos[j] < m);
      }

      ordered_ness = fast::max(ordered_ness, order);

    } else {
      pos[i] = INT_MAX;
    }
  }

  score += ordered_ness * Params::Ordered + cnt * Params::AnyHit;

  return score;
}

static double rank_isrs(const vector<isr*> words, Offset start, Offset end, size_t rare) {
  for (const auto isr : words) {
    isr->seek(start);
  }

  vector<Offset> last(words.size(), MAX_OFFSET);
  vector<size_t> counts(words.size(), 0);

  double score = 0;

  while (!words[rare]->is_end() && words[rare]->offset() < end) {
    counts[rare]++;
    last[rare] = words[rare]->offset();

    // get as close as possible
    for (size_t i = 0; i < words.size(); ++i) {
      if (i != rare) {
        while (!words[i]->is_end() && words[i]->offset() < last[rare]) {
          counts[i]++;
          last[i] = words[i]->offset();
          words[i]->next();
        }

        if (!words[i]->is_end() && words[i]->offset() < end && (last[i] == MAX_OFFSET || last[rare] - last[i] > words[i]->offset() - last[rare])) {
            last[i] = words[i]->offset();
        }
      }
    }

    const auto decay = pow(Params::Decay, last[rare] - start);

    size_t word_in_span = 1;
    Offset span_begin = last[rare], span_end = last[rare];

    for (size_t i = 0; i < words.size(); ++i) {
      if (last[i] == MAX_OFFSET || i == rare) continue;

      ++word_in_span;
      span_begin = min(span_begin, last[i]);
      span_end = max(span_end, last[i]);

      const auto width = max(max(last[i], last[rare]) - min(last[i], last[rare]), Offset(1));
      const auto ordered_double = (
        (i < rare && last[i] <= last[rare]) ||
        (i > rare && last[i] >= last[rare])
      );

      if (ordered_double) {
        score += Params::ODouble / width / decay;
      } else {
        score += Params::Double / width / decay;
      }
      
      for (size_t j = i + 1; j < words.size(); ++j) {
        if (last[j] == MAX_OFFSET || j == rare) continue;
        
        const auto width = max(max(last[i], max(last[j], last[rare])) - 
                           min(last[i], min(last[j], last[rare])), Offset(1));

        const auto ordered_trip = (
          (j < rare && last[j] <= last[rare] && last[i] <= last[j]) ||
          (j > rare && last[j] >= last[rare] && ordered_double)
        );

        if (ordered_trip) {
          score += Params::OTriple / width / decay;
        } else {
          score += Params::Triple / width / decay;
        }

      }
    }

    if (span_begin < span_end && word_in_span == words.size()) {
      const auto width = span_end - span_begin;
      score += Params::SpanMult / width / decay;
    }

    words[rare]->next();
  }

  return score;
}

void rank(const hashblob *blob, const vector<string_view> &flat,
                 isr_container *matches,
                 array<Result, MAX_RESULTS> &results) {
  if (flat.size() == 0)
    return;

  vector<isr*> body;
  vector<isr*> title;
  size_t rare_idx = 0;
  size_t rare_count = 0;

  for (const auto word : flat) {
    auto pl = blob->get(word);
    if (pl) {
      body.push_back(pl->get_isr());
      if (rare_count == 0 || rare_count > pl->words()) {
        rare_count = pl->words();
        rare_idx = body.size() - 1;
      }
    } else {
      body.push_back(new isr_null);
    }

    pl = blob->get(string(word) + "#");
    if (pl) {
      title.push_back(pl->get_isr());
    } else {
      title.push_back(new isr_null);
    }
  }

  while (!matches->is_end()) {
    const auto body_score = rank_isrs(body, matches->get_doc_start(),
                                      matches->get_doc_end(), rare_idx);
    const auto title_score = rank_isrs(title, matches->get_doc_start(),
                                       matches->get_doc_end(), rare_idx);

    const auto url = matches->get_doc_url();
    auto score = body_score + title_score * Params::Mtitle +
                 url_rank(url, flat, rare_idx);

    score += Params::is_good_doc_len(matches->get_doc_end() - matches->get_doc_start()) * Params::GoodLength;

    insertion_sort(results, {score, url});

    matches->next();
  }

  for (const auto isr : body) {
    delete isr;
  }

  for (const auto isr : title) {
    delete isr;
  }

  return;
}

}  // namespace fast::query
