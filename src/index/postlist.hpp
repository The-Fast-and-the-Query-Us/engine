#include "list.hpp"
#include <cstddef>
#include <hashtable.hpp>

namespace fast {
class postlist {

  public:

  static size_t size_needed(const list<hashtable::post> &posts) {
    (void)posts;
    return 0; // TODO
  }

  // write posting list and return its end
  static char *write(const list<hashtable::post> &posts, char *buffer) {
    (void)posts;
    return buffer;
  }
};
}
