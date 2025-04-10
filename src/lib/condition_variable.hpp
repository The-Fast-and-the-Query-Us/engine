#pragma once

#include "mutex.hpp"
#include <pthread.h>

namespace fast {

class condition_variable {
public:
  condition_variable() { pthread_cond_init(&cv, nullptr); }

  ~condition_variable() { pthread_cond_destroy(&cv); }

  void wait(fast::mutex *m) { pthread_cond_wait(&cv, &m->m); }

  void signal() { pthread_cond_signal(&cv); }

  void broadcast() { pthread_cond_broadcast(&cv); }

private:
  pthread_cond_t cv;
};

} // namespace fast
