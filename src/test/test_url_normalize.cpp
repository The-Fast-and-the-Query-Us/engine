#include <url_parser.hpp>
#include <string.hpp>
#include <pair.hpp>
#include <cassert>

using namespace fast;

fast::pair<string> tests[] = {
  {"https://feministstudies.ucsc.edu/undergraduate/feminist-studies-major/../../graduate/../courses/../graduate/designated-emphasis/index.html", 
  "https://feministstudies.ucsc.edu/graduate/designated-emphasis/index.html"}
};

int main() {
  
  for (auto &p : tests) {
    crawler::url_parser::resolve_relative(p.first);
    assert(p.first == p.second);
  }
}
