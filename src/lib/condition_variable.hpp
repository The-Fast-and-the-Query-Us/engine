#pragma once

#include "mutex.hpp"
#include <pthread.h>

namespace fast {

class condition_variable {
public:
  condition_variable() = default;

  ~condition_variable() { pthread_cond_destroy(&cv_); }

  void wait(fast::mutex *m) { pthread_cond_wait(&cv_, &m->m); }

  void signal() { pthread_cond_signal(&cv_); }

  void broadcast() { pthread_cond_broadcast(&cv_); }

private:
  pthread_cond_t cv_ = PTHREAD_COND_INITIALIZER;
};

} // namespace fast
