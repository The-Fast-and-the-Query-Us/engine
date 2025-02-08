#include "bloom_filter.hpp"
#include <cassert>
#include <iostream>
#include <string>

int main() {
  fast::bloom_filter<std::string> bf(100, 0.01);

  bf.insert("hello");

  assert(bf.contains("hello"));
  assert(!bf.contains("hell"));
  std::cout << "PASS string basic" << std::endl << std::endl;

  const double FPR_SMALL = 0.01;
  fast::bloom_filter<int> bf1(10000, FPR_SMALL);
  for (int i = 0; i < 10000; i++) {
    if (i % 2) {
      bf1.insert(i);
    }
  }

  std::cout << "TESTING int FPR SMALL:" << std::endl;
  double emp_fpr = 0.0;
  for (int i = 0; i < 10000; i++) {
    if (!(i % 2)) {
      bool res = bf1.contains(i);
      emp_fpr += res;
    }
  }

  std::cout << "expected fpr: " << std::to_string(FPR_SMALL) << std::endl;
  std::cout << "empirical fpr: " << std::to_string(emp_fpr / 10000) << std::endl
            << std::endl;

  const double FPR_LARGE = 0.5;
  fast::bloom_filter<int> bf2(10000, FPR_LARGE);
  for (int i = 0; i < 10000; i++) {
    if (i % 2) {
      bf2.insert(i);
    }
  }

  std::cout << "TESTING int FPR_LARGE:" << std::endl;
  emp_fpr = 0.0;
  for (int i = 0; i < 10000; i++) {
    if (!(i % 2)) {
      bool res = bf2.contains(i);
      emp_fpr += res;
    }
  }

  std::cout << "expected fpr: " << std::to_string(FPR_LARGE) << std::endl;
  std::cout << "empirical fpr: " << std::to_string(emp_fpr / 10000)
            << std::endl;

  return 0;
}
