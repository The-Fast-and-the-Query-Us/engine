#include <csignal>
#include <cstring>
#include <iostream>
#include "crawler.hpp"

// Global pointer for signal handler
using namespace fast::crawler;

crawler* g_crawler = nullptr;

void signal_handler(int signum) {
  if (signum == SIGINT && g_crawler) {
    std::cout << "SIGINT: graceful shutdown\n";
    g_crawler->shutdown();
  }
}

int main() {
  struct sigaction sa{};
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_handler;
  sigaction(SIGINT, &sa, nullptr);

  try {
    crawler c;
    g_crawler = &c;
    c.run();
    g_crawler = nullptr;
  } catch (const std::exception& e) {  // std::runtime_error
    g_crawler = nullptr;
    std::cerr << "Error: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
