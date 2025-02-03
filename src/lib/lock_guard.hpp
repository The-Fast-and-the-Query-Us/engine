#include <stdexcept>
#include <pthread.h>

namespace fast {

class lock_guard {
  public:
    lock_guard() {
      if (pthread_mutex_init(&m, nullptr) != 0) {
        throw std::runtime_error("pthread mutex unable to init\n");
      }
      pthread_mutex_lock(&m);
    }

    lock_guard(const lock_guard&) = delete;
    lock_guard& operator=(const lock_guard&) = delete;

    lock_guard(lock_guard&& other) noexcept {
      m = other.m;
      pthread_mutex_init(&other.m, nullptr); // Avoiding double destruction
    }

    lock_guard& operator=(lock_guard&& other) noexcept {
      if (this != &other) {
        pthread_mutex_unlock(&m);
        pthread_mutex_destroy(&m);
        m = other.m;
        pthread_mutex_init(&other.m, nullptr);
      }
      return *this;
    }

    ~lock_guard() {
      pthread_mutex_unlock(&m);
      pthread_mutex_destroy(&m);
    }

  private:
    pthread_mutex_t m;
};
  
}
