// This file is for testing with data from the python script

#include <cstring>
#include <hashtable.hpp>
#include <iostream>

char path[5000];
char *path_end;

void write(const fast::hashtable *ht) {
  static int next = 0;
}

int main(int argc, char **argv) {
  auto ht =  new fast::hashtable;

  strcpy(path, argv[1]);
  path_end = path + strlen(path);
  *path_end = '/';
  ++path_end;

  int in;
  while (std::cin >> in) {

  }

  write(ht);
  delete ht;
}
