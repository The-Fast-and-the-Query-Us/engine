#include <cassert>
#include <frontier.hpp>
using namespace fast;
using namespace crawler;

constexpr char SAVE_PATH[] = "./frontier_test.txt";
int main() {
  frontier f(SAVE_PATH);
  string psql_wiki = "https://en.wikipedia.org/wiki/PostgreSQL/";
  f.insert(psql_wiki);

  string amazon = "https://www.amazon.com/";
  f.insert(amazon);

  assert(f.next() == frontier::extract_hostname(psql_wiki));
  auto actual = f.next();
  std::cout << "actual: " << actual.begin() << '\n';
  std::cout << "expected: " << frontier::extract_hostname(amazon).begin()
            << '\n';
  assert(actual == frontier::extract_hostname(amazon));
  f.insert(amazon);

  f.save();

  // frontier f_loaded(SAVE_PATH);
  // f_loaded.load();
  // auto actual = f_loaded.next();
  // std::cout << "actual: " << actual.begin() << '\n';
  // std::cout << "expected: " << frontier::extract_hostname(amazon).begin()
  //           << '\n';
  // assert(f_loaded.next() == frontier::extract_hostname(amazon));
  return 0;
}
