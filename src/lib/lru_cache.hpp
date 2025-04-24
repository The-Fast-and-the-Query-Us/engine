#pragma once
#include "flat_map.hpp"
#include "pair.hpp"
#include <list>
#include <utility>
namespace fast {

template <typename K, typename V> class lru_cache {
  using list_iter = typename std::list<pair<K, V>>::iterator;
  flat_map<K, list_iter> mp;
  std::list<pair<K, V>> lst;
  size_t cap, sz;

  void del_lru() {
    const auto &victim = lst.back();
    mp.remove(*victim.first);
    lst.erase(victim);
    --sz;
  }

  void insert(K key, V val) {
    lst.push_front({key, val});
    mp[key] = lst.begin();
    ++sz;
    if (sz > cap)
      del_lru();
  }

  void touch(K key) {
    pair<K, V> p = std::move(*mp[key]);
    lst.erase(mp[key]);
    lst.push_front(std::move(p));
    mp[key] = lst.begin();
  }

public:
  class iterator {
    list_iter it;

  public:
    explicit iterator(list_iter _it) : it{_it} {}
    bool operator==(const iterator &other) { return other.it == it; }
    bool operator!=(const iterator &other) { return !(*this == other); }

    V &operator*() const { return *it; }

    iterator &operator++() {
      ++it;
      return *this;
    }
  };

  lru_cache(size_t capacity) : cap{capacity}, sz{0} {}

  bool contains(K key) { return mp.find(key) != nullptr; }

  iterator begin() { return iterator(lst.begin()); }

  iterator end() { return iterator(lst.end()); }

  iterator insert(pair<K, V> p) {
    lst.push_front(p);
    mp[p.first] = lst.begin();
    ++sz;
    if (sz > cap)
      del_lru();
    return iterator(lst.begin());
  }

  iterator find(K key) {
    if (!contains(key))
      return iterator(lst.end());
    touch(key);
    return iterator(mp[key]);
  }

  void erase(K key) {
    if (!contains(key))
      return;
    lst.erase(mp[key]);
    mp.remove(key);
    --sz;
  }

  V &operator[](K key) {
    if (!contains(key))
      insert({key, V()});
    touch(key);
    return *mp[key];
  }
};

} // namespace fast
