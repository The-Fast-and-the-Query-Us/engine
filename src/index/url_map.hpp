#pragma once

#include "common.hpp"
#include "compress.hpp"
#include "list.hpp"
#include "string.hpp"
#include "hashtable.hpp"
#include "string_view.hpp"
#include <cstring>

namespace fast {

// list of Urls should not be empty!
class url_map {
  size_t num_urls, len;

  size_t *buckets() {
    return &len + 1;
  }

  const size_t *buckets() const {
    return &len + 1;
  }

  unsigned char *data() {
    return reinterpret_cast<unsigned char *>(buckets() + num_urls);
  }

  const unsigned char *data() const {
    return reinterpret_cast<const unsigned char *>(buckets() + num_urls);
  }

  static void init_buckets(const list<Url> &urls, url_map *buffer) {
    buffer->num_urls = urls.size();

    for (size_t i = 0; i < urls.size(); ++i) {
      buffer->buckets()[i] = 0;
    }

    for (const auto &url : urls) {
      buffer->buckets()[url.second % urls.size()] +=
        sizeof(url.second) + url.first.size() + encoded_size(url.first.size());
    }

    size_t acc = 0;

    for (size_t i = 0; i < urls.size(); ++i) {
      auto &bucket = buffer->buckets()[i];
      swap(acc, bucket);
      acc += bucket;
    }

    buffer->len = acc;
  }

public:
  static size_t size_needed(const list<Url> &urls) {
    size_t dynamic = 0;

    dynamic += urls.size() * sizeof(size_t);

    for (const auto &url : urls) {
      dynamic +=
        sizeof(url.second) + url.first.size() + encoded_size(url.first.size());
    }

    return dynamic + sizeof(url_map);
  }

  static unsigned char *write(const list<Url> &urls, url_map *buffer) {
    init_buckets(urls, buffer);

    for (const auto &url : urls) {
      auto &offset = buffer->buckets()[url.second % urls.size()];
      const auto data = buffer->data();

      write_unaligned(url.second, data + offset);
      offset += sizeof(url.second);

      encode(url.first.size(), data + offset);
      offset += encoded_size(url.first.size());

      memcpy(data + offset, url.first.c_str(), url.first.size());
      offset += url.first.size();
    }

    init_buckets(urls, buffer);
    return buffer->data() + buffer->len;
  }

  // number of urls in this list
  size_t count() const { return num_urls; }

  // get url for offset or empty string_view if there isnt one
  const string_view get(uint64_t offset) const {
    const auto bucket = buckets()[offset % num_urls];

    auto start = data() + bucket;
    const auto end = (offset % num_urls == num_urls - 1) ? 
      data() + len : data() + buckets()[(offset + 1) % num_urls];

    while (start < end) {
      const auto key = read_unaligned<Offset>(start);
      start += sizeof(Offset);

      uint64_t tmp;
      start = decode(tmp, start);

      if (key == offset) {
        return string_view((char*)start, tmp);
      } else {
        start += tmp;
      }
    }

    return string_view();
  }
};

}
