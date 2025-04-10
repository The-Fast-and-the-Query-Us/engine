#include <cassert>
#include <english.hpp>
#include <string.hpp>

using namespace fast::english;
using namespace fast;

void test(const char* in, const char* out) {
  string str(in);
  porter_stem(str);
  assert(str == out);
}

int main() {
  test("multidimensional", "multidimension");
  test("characterization", "character");
  test("flies", "fli");
  test("denied", "deni");
  test("feed", "feed");
  test("agreed", "agre");
  test("milling", "mill");
  test("messing", "mess");
  test("sadly", "sadli");
  test("goodness", "good");
  test("best", "best");
  test("better", "better");
  test("", "");
  test("try", "try");
  test("by", "by");
  test("sky", "sky");
  test("frivolously", "frivol");
}
