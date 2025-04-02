#include <cassert>
#include <cstdint>
#include <string>
#include <vector>

enum operation_t : uint8_t {
  WORD = 0,
  AND = 1,
  OR = 2,
  PHRASE = 3,
};

struct query {
  operation_t op;
  // only used for operation WORD
  std::string word;
  isr reader;
  // used for AND, OR, or PHRASE
  query* lhs{nullptr};
  query* rhs{nullptr};

  query() = delete;
  query(operation_t op, std::vector<std::string>& args) : op(op) {
    switch (op) {
      case WORD:
        assert(args.size() == 1);
        word = args[0];
        return;
      case AND:
      case OR:
      case PHRASE: {
        assert(args.size() >= 2);
        auto last = std::vector<std::string>(1, args[args.size() - 1]);
        rhs = new query(WORD, last);
        args.pop_back();
        lhs = new query(args.size() == 1 ? WORD : op, args);
      }
    }
  }

  int next() {
    switch (op) {
      case WORD:
        return ++isr;
      case OR:
        while (lhs.isr.curr() < rhs.isr.curr() /* and in diff documents */) {
          ++lhs;
        }
        while (rhs.isr.curr() < lhs.isr.curr() /* and in diff documents */) {
          ++rhs;
        }
        return lhs.isr.curr();
    }
  }
};
