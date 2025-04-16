#include <frontier.hpp>
#include <string.hpp>
#include <iostream>
#include "crawler.hpp"
#include "english.hpp"

int main() {
  fast::string url = "https://amazon.com/?search='hellomf'/bruh";
  std::cout << fast::crawler::frontier::extract_hostname(url).begin() << '\n';
  std::cout << fast::english::strip_url_prefix(url).begin() << '\n';
  return 0;
}
