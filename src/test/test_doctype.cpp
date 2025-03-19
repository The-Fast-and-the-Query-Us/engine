#include "compress.hpp"
#include "string_view.hpp"
#include "types.hpp"
#include <cassert>
#include <iostream>
#include <ostream>

using namespace fast::index;

int main() {
  fast::string_view sv = "some text";

  post<Doc> p(sv, 0);

  const auto space = p.size_required(0);

  unsigned char buffer[64];

  const auto end = p.write(buffer, 0);

  assert(end == buffer + space);

  uint64_t data;

  const unsigned char * ptr = buffer;
  ptr = fast::decode(data, ptr);

  std::cout << data << std::endl;

  ptr = fast::decode(data, ptr);

  std::cout << data << std::endl;

  isr<Doc> is(buffer, 0);

  assert(is.url() == sv);
}
