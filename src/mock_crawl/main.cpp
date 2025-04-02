// This file is for testing with data from the python script

#include <cstring>
#include <hashtable.hpp>
#include <iostream>
#include <string>
#include <sys/fcntl.h>
#include <hashblob.hpp>
#include <sys/mman.h>
#include <unistd.h>

char path[5000];
char *path_end;

// max hashtable size
constexpr int MAX_SZ = 50'000;

void write(const fast::hashtable *ht) {
  std::cout << "Writing map" << std::endl;

  static int next = 0;
  sprintf(path_end, "%d", next);
  ++next;

  const int fd = open(path, O_CREAT | O_RDWR, 0777);
  if (fd == -1) {
    perror("File open fail");
    exit(1);
  }

  const auto space = fast::hashblob::size_needed(*ht);

  if (ftruncate(fd, space) == -1) {
    perror("resize failed");
    exit(1);
  }

  auto mptr = mmap(NULL, space, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
  if (mptr == MAP_FAILED) {
    perror("cant mmap");
    close(fd);
    exit(1);
  }

  auto blob = (fast::hashblob*) mptr;
  fast::hashblob::write(*ht, blob);

  munmap(mptr, space);
  close(fd);

  std::cout << "Map written" << std::endl;
}

int main(int argc, char **argv) {
  auto ht =  new fast::hashtable;

  strcpy(path, argv[1]);
  path_end = path + strlen(path);
  *path_end = '/';
  ++path_end;

  int in;
  while (std::cin >> in) {
    std::string word;
    std::cin >> word;

    if (in == 0) { // url
      ht->add_doc(word.c_str());

      if (ht->tokens() >= MAX_SZ) {
        write(ht);
        delete ht;
        ht = new fast::hashtable;
      }

    } else if (in == 1) { // word
      ht->add(word.c_str());
    }
  }

  if (ht->tokens() > 0) {
    write(ht);
  }

  delete ht;
}
