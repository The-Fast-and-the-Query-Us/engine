#include "list.hpp"
#include <cmath>
#include <cstddef>
#include <hashtable.hpp>

namespace fast {
/*
* Format
* numOccurances
* Sync table: < doc delta, post delta, offset in posttable> , len == sqrt(len(post))
*   - Terminated by 0x0000 because stored as (doc, offset)
* Post table: < doc delta <post deltas> >
*   - Terminated by 0x00 (doc must be different)
*/
class postlist {
  public:

  static size_t size_needed(const list<hashtable::post> &posts) {
    (void) posts;

    return sizeof(size_t) + 0 + 0; // TODO
  }

  // write posting list and return its end
  static char *write(const list<hashtable::post> &posts, char *buffer) {
    (void)posts;
    return buffer;
  }
};
}
