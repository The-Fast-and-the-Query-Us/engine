#include <pthread.h>
#include "mutex.hpp"

namespace fast {

  class cv {
    public:
      cv() = default;

      ~cv() {
        pthread_cond_destroy(&cv_);
      }

      void wait(fast::mutex *m) {
        pthread_cond_wait(&cv_, &m->m);
      }

      void signal() {
        pthread_cond_signal(&cv_);
      }

      void broadcast() {
        pthread_cond_broadcast(&cv_);
      }

    private:
      pthread_cond_t cv_ = PTHREAD_COND_INITIALIZER;
  };

}
