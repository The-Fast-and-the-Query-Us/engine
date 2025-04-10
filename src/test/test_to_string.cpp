#include <string.hpp>
#include <string>

using namespace fast;

int main() {
  for (uint64_t i = 0; i < 9999; ++i) {
    const auto mine = to_string(i);
    const auto stl = std::to_string(i);
    assert(mine.size() == stl.size());
    for (size_t j = 0; j < mine.size(); ++j) {
      assert(mine[j] == stl[j]);
    }
  }
}
