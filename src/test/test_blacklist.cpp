#include <crawler.hpp>
#include <cstddef>
#include <iostream>
#include <cassert>
#include "string.hpp"


int main() {
  fast::pair<fast::string, bool> test_urls[] = {
    {"lskhflksdflsdk.#.slkfdjloginskldjf", false},
    {"www.hello.html?query=login", true},
    {"https://www.nytimes.com", false},
    {"cn.nyt.com", true},
    {"data.cdn#title", true},
    {"wtfyoushearchingthissite.com/?id=79/?hello", false},
    {"es.wikipedia.com", true}
  };

  for (const auto &pair : test_urls) {
    std::cout << pair.first.begin() << '\n';
    assert(fast::crawler::crawler::is_blacklisted(pair.first) == pair.second);
  }

  return 0;
}
