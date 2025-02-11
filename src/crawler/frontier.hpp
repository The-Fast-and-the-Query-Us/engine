#pragma once

#include "../lib/condition_variable.hpp"
#include "../lib/queue.hpp"
#include "../lib/scoped_lock.hpp"
#include "../lib/string.hpp"
#include "../lib/vector.hpp"

#include <cstdint>
#include <unordered_map>
#include <vector>

namespace fast {
namespace crawler {

class frontier {
public:
  frontier() {
    // need to add functionality to read in previous stuff from memory
  }

  void insert(fast::string url) {
    scoped_lock lock(&mtx);

    fast::string hostname;
    uint8_t slash_cnt = 0;
    for (size_t i = 0; i < url.size(); ++i) {
      if (url[i] == '/' && ++slash_cnt == 3) {
        break;
      }
      hostname[i] += url[i];
    }

    priorities[calc_priority(url)].push(url);

    ++num_links;
    cv.signal();
  };

  fast::string next() {
    scoped_lock lock(&mtx);

    while (num_links == 0)
      cv.wait(&mtx);

    for (size_t i = 0; i < priorities.size(); i++) {
      fast::queue<fast::string> &curr_pri = priorities[i];

      if (!curr_pri.empty()) {

        for (size_t i = 0; i < curr_pri.size(); ++i) {

          fast::string curr = curr_pri.front();
          curr_pri.pop();

          if (crawl_cnt[curr] <= CRAWL_LIM) {
            --num_links;
            return curr;
          }

          curr_pri.push(curr);
        }
      }
    }

    return "";
  };

private:
  static constexpr uint8_t GOOD_LEN = 25;

  static constexpr uint8_t CRAWL_LIM = 3;

  fast::mutex mtx;

  fast::condition_variable cv;

  uint64_t num_links;

  fast::vector<fast::queue<fast::string>> priorities;

  std::unordered_map<fast::string, uint8_t> crawl_cnt;

  std::vector<fast::string> good_tld = {"com", "org", "gov", "net", "edu"};

  uint8_t calc_priority(fast::string hostname) {
    uint8_t score = 0;

    // +1 point for good length
    score += hostname.size() <= GOOD_LEN;

    size_t idx = 0;

    // +1 point for secure protocol
    fast::string protocol;
    while (hostname[idx] != ':' && idx < hostname.size())
      protocol += hostname[idx];

    // scan from back
    idx = hostname.size() - 1;

    // +1 point for tld
    fast::string tld;
    while (hostname[idx] != '.' && idx > 0)
      tld += hostname[idx];

    for (auto d : good_tld) {
      if (tld == d) {
        ++score;
        break;
      }
    }

    return score;
  }
};

} // namespace crawler
} // namespace fast
