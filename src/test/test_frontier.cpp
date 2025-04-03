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

  f.save();

  frontier f_loaded(SAVE_PATH);
  f.load();
  assert(f_loaded.next() == frontier::extract_hostname(amazon));
  return 0;
}
