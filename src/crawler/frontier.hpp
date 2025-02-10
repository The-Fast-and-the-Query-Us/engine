#pragma once

#include "condition_variable.hpp"
#include "mutex.hpp"
#include "queue.hpp"
#include "scoped_lock.hpp"
#include "string.hpp"
#include "vector.hpp"

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
    fast::string hostname;
    for (size_t i = 0; i < url.size(); ++i) {
      if (url[i] == '/') {
        break;
      }
      hostname[i] += url[i];
    }

    priorities[calc_priority(url)].push(url);
  };

  fast::string next() { fast::string *head; };

private:
  static constexpr uint8_t GOOD_LEN = 25;

  fast::mutex mtx;

  fast::condition_variable cv;

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
