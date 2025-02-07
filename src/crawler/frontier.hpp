#pragma once

#include "../lib/queue.hpp"
#include "../lib/string.hpp"

class frontier {
  public:
    frontier() {
      // need to add functionality to read in previous stuff from memory
    }


  private:
    fast::queue<fast::string> top_priority;
    fast::queue<fast::string> mid_priority;
    fast::queue<fast::string> low_priority;
};
