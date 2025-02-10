#pragma once

#include <pthread.h>
#include <stdexcept>

namespace fast {

class mutex {
public:
  mutex() {
    if (pthread_mutex_init(&m, nullptr) != 0) {
      throw std::runtime_error("pthread mutex unable to init\n");
    }
  }

  mutex(const mutex &) = delete;
  mutex &operator=(const mutex &) = delete;

  mutex(mutex &&other) noexcept {
    m = other.m;
    pthread_mutex_init(&other.m, nullptr); // Avoiding double destruction
  }

  mutex &operator=(mutex &&other) noexcept {
    if (this != &other) {
      pthread_mutex_unlock(&m);
      pthread_mutex_destroy(&m);
      m = other.m;
      pthread_mutex_init(&other.m, nullptr);
    }
    return *this;
  }

  void lock() { pthread_mutex_lock(&m); }

  void unlock() { pthread_mutex_unlock(&m); }

  ~mutex() { pthread_mutex_destroy(&m); }

  pthread_mutex_t m;
};

} // namespace fast
