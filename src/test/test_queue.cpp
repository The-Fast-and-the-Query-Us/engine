#include <queue>
#include "queue.hpp"

using namespace fast;

constexpr int ITERATIONS = 1000;

int main() {
  queue<int> q1;
  std::queue<int> q2;

  for (int i = 0; i < ITERATIONS; ++i) {
    q1.push(i);
    q2.push(i);

    assert(q1.front() == q2.front());
    assert(q1.back() == q2.back());
    if (i % 10 == 0) {
      q1.pop();
      q2.pop();
    }
  }

  while (!q1.empty()) {
    assert(q1.front() == q2.front());
    q1.pop();
    q2.pop();
  }
}
