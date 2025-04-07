#include <atomic>
#include <string.hpp>
#include <url_sender.hpp>

using namespace fast::crawler;

const fast::string ips = "test_data/loopback_ip.txt";
const fast::string urls[] = {"www,google.com", "www.github.com", "www.walmart.com"};

std::atomic_int32_t idx = 0;

void test(const fast::string &url) {
  const auto tid = idx.fetch_add(1);
  assert(url == urls[tid]);
}


int main() {
  url_sender sender(ips, test);
  for (const auto &url : urls) {
    sender.send_link(url);
  }
}
