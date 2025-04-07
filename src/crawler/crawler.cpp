#include "crawler.hpp"
#include <sys/signal.h>
#include <csignal>
#include <iostream>

fast::crawler::crawler c{};

void handle_interrupt(int sig) {
  if (sig == SIGINT || sig == SIGSTOP) {
    std::cout << "graceful shutdown...\n";
    c.shutdown();
  }
}

int main() {
  struct sigaction sa{};
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = handle_interrupt;
  sigaction(SIGINT, &sa, nullptr);
  sigaction(SIGSTOP, &sa, nullptr);

  c.run();
  return 0;
}
