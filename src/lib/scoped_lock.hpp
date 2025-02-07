#pragma once

#include "mutex.hpp"

namespace fast {
  class scoped_lock {
    public:
      scoped_lock(fast::mutex *m_) : m(m_) {
        m->lock();
      }

      ~scoped_lock() {
        m->unlock();
      }

    private:
      fast::mutex *m;
  };



}
