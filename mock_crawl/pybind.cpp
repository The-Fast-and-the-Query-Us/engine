#include "hashblob.hpp"
#include <pybind11/pybind11.h>
#include <hashtable.hpp>
#include <string_view.hpp>
#include <string>
#include <sys/fcntl.h>
#include <sys/mman.h>
#include <unistd.h>

fast::hashtable *ht;

void alloc() {
  ht = new fast::hashtable;
}

void erase() {
  delete ht;
}

void add_word(const std::string &word) {
  const fast::string_view sv(word.data(), word.size());
  ht->add(sv);
}

void add_url(const std::string &url) {
  const fast::string_view sv(url.data(), url.size());
  ht->add_doc(sv);
}

size_t get_word_count() {
  return ht->tokens();
}

void write_blob(const std::string &path) {
  const auto fd = open(path.c_str(), O_CREAT | O_RDWR, 0777);

  if (fd == -1) {
    perror("open failed");
    exit(1);
  }

  const auto space = fast::hashblob::size_needed(*ht);

  if (ftruncate(fd, space) == -1) {
    perror("Ftruncate fail");
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
}

PYBIND11_MODULE(pybind, m) {
  m.def("alloc", &alloc, "Allocate new hashtable");
  m.def("erase", &erase, "delete hashtable");
  m.def("add_word", &add_word);
  m.def("add_url", &add_url);
  m.def("num_tokens", &get_word_count);
  m.def("write_blob", &write_blob);
}
