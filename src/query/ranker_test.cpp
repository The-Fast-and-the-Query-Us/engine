#include "constants.hpp"
#include "ranker.hpp"
#include <cassert>

using namespace fast;

int main(int argc, char **argv) {
  assert(argc == 2);

  fast::array<fast::query::Result, fast::query::MAX_RESULTS> results;

  std::function<void(const fast::query::Result&)> call_back = [&results](const fast::query::Result &res) {
    if (results[query::MAX_RESULTS - 1].first < res.first) {
      results[query::MAX_RESULTS - 1] = res;
      for (size_t i = query::MAX_RESULTS - 1; i > 0; --i) {
        if (results[i].first > results[i - 1].first) {
          swap(results[i], results[i - 1]);
        } else {
          break;
        }
      }
    }
  };

  query::rank_all(argv[1], call_back);

  for (const auto &res : results) {
    std::cout << res.second.c_str() << " score: " << res.first << std::endl;
  }
}
