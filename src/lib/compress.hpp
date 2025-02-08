#pragma once

#include <cstddef>
namespace fast {

/*
* Return the number of bytes required to compress the given number
*/
template<class T>
size_t compressed_size(T number) {
  size_t ans{1};
  number >>= 7;
  while (number) number >>= 7, ++ans;
  return ans;
}

/*
* Write a number to a buffer with high bits coming first
*/
template<class T>
char* compress(T number, char* buffer) {
  for (auto byte = compressed_size(number) - 1; byte; --byte, ++buffer) {
    *buffer = (number >> (7 * byte)) | 0x80;
  }

  *buffer = (number) & ~(0x80);

  return buffer + 1;
}

/*
* Read bytes writen by 
*/
template <class T>
char* expand(T& number, char* buffer) {
  number = 0;
  do {
    number |= (*buffer & ~(0x80));
  } while (*(buffer++) & 0x80);
  return buffer;
}

}
