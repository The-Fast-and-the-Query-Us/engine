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
    num_links = 0;
    priorities.reserve(4);
  }

  frontier(const char *_save_path = nullptr) : save_path(_save_path) {
    num_links = 0;
    priorities.reserve(4);
  }

  void insert(fast::string url) {
    scoped_lock lock(&mtx);

    fast::string hostname;
    uint8_t slash_cnt = 0;
    for (size_t i = 0; i < url.size(); ++i) {
      if (url[i] == '/' && ++slash_cnt == 3) {
        break;
      }
      hostname += url[i];
    }

    priorities[calc_priority(hostname)].push(url);
    ++crawl_cnt[url];

    ++num_links;
    cv.signal();
  };

  fast::string next() {
    scoped_lock lock(&mtx);

    while (num_links == 0)
      cv.wait(&mtx);

    for (size_t i = 0; i < priorities.size(); ++i) {
      fast::queue<fast::string> &curr_pri = priorities[i];

      if (!curr_pri.empty()) {

        const size_t n = curr_pri.size();
        for (size_t j = 0; j < n; ++j) {

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

  static constexpr uint8_t TLD_LEN = 3;

  fast::mutex mtx;

  fast::condition_variable cv;

  uint64_t num_links;

  const char *save_path;

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
    while (idx < hostname.size() && hostname[idx] != ':')
      protocol += hostname[idx];

    score += protocol == "https";

    // scan from back
    idx = hostname.size() - 1;

    // +1 point for tld
    fast::string tld;
    while (idx > 0 && hostname[idx] != '.')
      tld += hostname[idx];

    tld.reverse(0, tld.size() - 1);

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
