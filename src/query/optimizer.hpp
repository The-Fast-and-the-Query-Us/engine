#pragma once

#include <cstdlib>
#include <functional>
#include <unordered_map>
#include "array.hpp"
#include "constants.hpp"
#include "ranker.hpp"

static constexpr size_t NUM_PARAMS = 10;
using params_t = fast::array<double, NUM_PARAMS>;

namespace fast {
class gradient_descent {
  static constexpr double ALPHA = 0.01;
  static constexpr double DELTA = 800;
  static constexpr double EPS = 0.1;
  std::vector<std::string> queries;
  std::vector<std::vector<std::string>> ddg_results;

  static std::unordered_map<std::string, int> get_ranker_results(
      const std::string& query) {
    std::vector<query::Result> results_vec;
    auto callback = [&results_vec](const query::Result& result) {
      results_vec.push_back(result);
    };
    query::rank_all(query.data(), callback);

    std::vector<std::pair<double, std::string>> vec;
    vec.reserve(results_vec.size());
    for (size_t i = 0; i < results_vec.size(); ++i) {
      vec.emplace_back(results_vec[i].first, results_vec[i].second.begin());
    }

    std::sort(vec.begin(), vec.end(), std::less<>{});

    std::unordered_map<std::string, int> results_map;
    for (size_t i = 0; i < vec.size(); ++i) {
      results_map[vec[i].second] = static_cast<int>(i + 1);
    }

    return results_map;
  }

  static double query_loss(const std::string& query,
                           const std::vector<std::string>& results) {
    auto ranker_results = get_ranker_results(query);

    double loss = 0;
    for (size_t i = 0; i < results.size(); ++i) {
      int expected = static_cast<int>(i + 1);
      int actual = ranker_results.contains(results[i])
                       ? ranker_results[results[i]]
                       : 100;
      int diff = expected - actual;

      loss += diff * diff;
    }

    return loss;
  }

  double loss() {
    double loss = 0;

    for (size_t i = 0; i < queries.size(); ++i) {
      loss += query_loss(queries[i], ddg_results[i]);
    }

    return loss;
  }

  params_t gradient() {
    params_t gradient{};
    for (size_t i = 0; i < NUM_PARAMS; ++i) {
      // std::cout << "changed factor from " << query::Params::FACTORS[i]
      //           << " to ";
      query::Params::FACTORS[i] += DELTA;
      // std::cout << query::Params::FACTORS[i] << '\n';
      double loss_plus = loss();
      // std::cout << "loss_plus: " << loss_plus << '\n';
      // std::cout << "changed factor from " << query::Params::FACTORS[i]
      //           << " to ";
      query::Params::FACTORS[i] -= 2 * DELTA;
      // std::cout << query::Params::FACTORS[i] << '\n';
      double loss_minus = loss();
      query::Params::FACTORS[i] += DELTA;
      // std::cout << "loss_minus: " << loss_minus << '\n';
      gradient[i] = (loss_minus - loss_plus) / (2 * DELTA);
    }
    return gradient;
  }

 public:
  gradient_descent() = delete;
  gradient_descent(const std::vector<std::string>& queries,
                   const std::vector<std::vector<std::string>>& ddg_results)
      : queries(queries), ddg_results(ddg_results) {}

  void run() {
    size_t i = 0;
    bool converged = false;
    while (!converged) {
      std::cout << "Iteration " << i++ << "\n\n";

      std::cout << "Title: " << fast::query::Params::FACTORS[0] << '\n';
      std::cout << "OrderedDouble: " << fast::query::Params::FACTORS[1] << '\n';
      std::cout << "Double: " << fast::query::Params::FACTORS[2] << '\n';
      std::cout << "OrderedTriple: " << fast::query::Params::FACTORS[3] << '\n';
      std::cout << "Triple: " << fast::query::Params::FACTORS[4] << '\n';
      std::cout << "Decay: " << fast::query::Params::FACTORS[5] << '\n';
      std::cout << "Span: " << fast::query::Params::FACTORS[6] << '\n';
      std::cout << "ShortUrl: " << fast::query::Params::FACTORS[7] << '\n';
      std::cout << "DomainHit: " << fast::query::Params::FACTORS[8] << '\n';
      std::cout << "UrlHit: " << fast::query::Params::FACTORS[9] << "\n\n";

      std::cout << "loss: " << loss() << '\n';

      params_t grad = gradient();

      converged = true;
      for (size_t j = 0; j < NUM_PARAMS; ++j) {
        std::cout << grad[j] << '\n';
        converged &= abs(ALPHA * grad[j]) < EPS;
        fast::query::Params::FACTORS[j] += ALPHA * grad[j];
      }
    }
  };
};
}  // namespace fast
