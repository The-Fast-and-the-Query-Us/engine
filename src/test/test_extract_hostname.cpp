#include <frontier.hpp>
#include <string.hpp>
#include <iostream>
#include "crawler.hpp"

int main() {
  fast::string url = "https://amazon.com/?search='hellomf'/bruh";
  std::cout << fast::crawler::frontier::extract_hostname(url).begin() << '\n';
  std::cout << fast::crawler::crawler::strip_url_protocol(url).begin() << '\n';
  return 0;
}
