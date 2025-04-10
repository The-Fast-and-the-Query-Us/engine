#include "optimizer.hpp"
#include "string.hpp"
#include "vector.hpp"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

int main() {
  std::string directory = "results-parsed";
  std::vector<std::string> queries;
  std::vector<std::vector<std::string>> ddg_results;

  for (const auto& entry : fs::directory_iterator(directory)) {
    if (entry.path().extension() == ".txt") {
      std::string filename = entry.path().filename().string();
      std::string query_name =
          filename.substr(0, filename.size() - 4);  // Remove ".txt"
      queries.push_back(query_name);

      std::vector<std::string> urls;
      std::ifstream infile(entry.path());
      std::string line;
      while (std::getline(infile, line)) {
        if (!line.empty()) {
          urls.push_back(line);
        }
      }
      ddg_results.push_back(urls);
    }
  }

  fast::gradient_descent gd(queries, ddg_results);
  gd.run();

  return 0;
}
