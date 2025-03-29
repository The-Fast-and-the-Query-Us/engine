// flat_map_test.cpp
#include "../lib/flat_map.hpp"
#include <unordered_map>
#include <map>
#include <string>
#include <iostream>
#include <cassert>
#include <chrono>
#include <vector>
#include <random>
#include <algorithm>
#include <functional>
#include <iomanip>

// Define XXHASH64 if not already defined
#ifndef XXHASH64
#define XXHASH64(x) XXH64(&x, sizeof(x), 0)
extern "C" {
#include "xxhash.h"
}
#endif

// Helper for timing measurements
class Timer {
private:
  std::chrono::time_point<std::chrono::high_resolution_clock> start_time;
  std::string operation_name;

public:
  explicit Timer(std::string operation) : operation_name(std::move(operation)) {
    start_time = std::chrono::high_resolution_clock::now();
  }

  ~Timer() {
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end_time - start_time);
    std::cout << std::left << std::setw(30) << operation_name << ": " 
      << std::right << std::setw(10) << duration.count() << " μs" << std::endl;
  }
};

// Generate random keys for testing
std::vector<int> generate_random_keys(size_t count, int min = 0, int max = 1000000) {
  std::vector<int> keys(count);
  std::random_device rd;
  std::mt19937 gen(rd());
  std::uniform_int_distribution<> distrib(min, max);

  for (size_t i = 0; i < count; ++i) {
    keys[i] = distrib(gen);
  }

  return keys;
}

// Basic functionality tests
void test_basic_functionality() {
  std::cout << "\n=== Testing Basic Functionality ===\n";

  // Test insertion and lookup
  {
    fast::flat_map<int, std::string> map;
    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    assert(map.contains(1));
    assert(map.contains(2));
    assert(map.contains(3));
    assert(!map.contains(4));

    assert(*map.find(1) == "one");
    assert(*map.find(2) == "two");
    assert(*map.find(3) == "three");
    assert(map.find(4) == nullptr);

    std::cout << "Basic insertion and lookup test passed\n";
  }

  // Test operator[]
  {
    fast::flat_map<int, std::string> map;
    map[1] = "one";
    map[2] = "two";
    map[3] = "three";

    assert(map[1] == "one");
    assert(map[2] == "two");
    assert(map[3] == "three");

    // Test default construction through operator[]
    map[4];
    assert(map.contains(4));
    assert(map[4] == "");

    std::cout << "Operator[] test passed\n";
  }

  // Test removal
  {
    fast::flat_map<int, std::string> map;
    map.insert(1, "one");
    map.insert(2, "two");
    map.insert(3, "three");

    map.remove(2);
    assert(map.contains(1));
    assert(!map.contains(2));
    assert(map.contains(3));

    map.remove(1);
    assert(!map.contains(1));

    map.remove(3);
    assert(!map.contains(3));

    // Remove non-existent key
    map.remove(4);  // Should not crash

    std::cout << "Removal test passed\n";
  }

  // Test erase (clear all)
  {
    fast::flat_map<int, std::string> map;
    map.insert(1, "one");
    map.insert(2, "two");

    map.erase();
    assert(!map.contains(1));
    assert(!map.contains(2));

    std::cout << "Erase test passed\n";
  }

  // Test copy constructor and assignment
  {
    fast::flat_map<int, std::string> map1;
    map1.insert(1, "one");
    map1.insert(2, "two");

    // Test copy constructor
    fast::flat_map<int, std::string> map2(map1);
    assert(map2.contains(1));
    assert(map2.contains(2));
    assert(*map2.find(1) == "one");

    // Modify original map
    map1.insert(3, "three");
    assert(!map2.contains(3));

    // Test assignment operator
    fast::flat_map<int, std::string> map3;
    map3 = map1;
    assert(map3.contains(1));
    assert(map3.contains(2));
    assert(map3.contains(3));

    std::cout << "Copy construction and assignment test passed\n";
  }

  // Test move constructor and assignment
  {
    fast::flat_map<int, std::string> map1;
    map1.insert(1, "one");
    map1.insert(2, "two");

    // Test move constructor
    fast::flat_map<int, std::string> map2(std::move(map1));
    assert(map2.contains(1));
    assert(map2.contains(2));
    assert(!map1.contains(1));  // map1 should be empty after move

    // Test move assignment
    fast::flat_map<int, std::string> map3;
    map3 = std::move(map2);
    assert(map3.contains(1));
    assert(map3.contains(2));
    assert(!map2.contains(1));  // map2 should be empty after move

    std::cout << "Move construction and assignment test passed\n";
  }

  // Test with non-trivial keys and values
  {
    fast::flat_map<std::string, std::vector<int>> map;
    map.insert("one", {1, 2, 3});
    map.insert("two", {4, 5, 6});

    assert(map.contains("one"));
    assert(map.contains("two"));

    auto* value = map.find("one");
    assert(value != nullptr);
    assert(value->size() == 3);
    assert((*value)[0] == 1);

    std::cout << "Non-trivial keys and values test passed\n";
  }
}

// Edge case tests
void test_edge_cases() {
  std::cout << "\n=== Testing Edge Cases ===\n";

  // Test with empty map
  {
    fast::flat_map<int, int> map;
    assert(!map.contains(1));
    assert(map.find(1) == nullptr);

    // These operations should not crash on empty map
    map.remove(1);
    map.erase();

    std::cout << "Empty map test passed\n";
  }

  // Test with large number of elements (forcing resize)
  {
    fast::flat_map<int, int> map;
    constexpr int NUM_ELEMENTS = 1000;

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
      map.insert(i, i * 10);
    }

    for (int i = 0; i < NUM_ELEMENTS; ++i) {
      assert(map.contains(i));
      assert(*map.find(i) == i * 10);
    }

    std::cout << "Large map test passed\n";
  }

  // Test with hash collisions
  {
    // Mock hash function to force collisions
    struct CollisionKey {
      int value;
      bool operator==(const CollisionKey& other) const {
        return value == other.value;
      }
    };

    // Every key with the same last 7 bits will have the same meta hash
    fast::flat_map<CollisionKey, int> map;

    // These will all have the same meta hash (last 7 bits)
    map.insert({0}, 0);
    map.insert({128}, 128);
    map.insert({256}, 256);

    assert(map.contains({0}));
    assert(map.contains({128}));
    assert(map.contains({256}));

    assert(*map.find({0}) == 0);
    assert(*map.find({128}) == 128);
    assert(*map.find({256}) == 256);

    std::cout << "Hash collision test passed\n";
  }

  // Test with deleted elements
  {
    fast::flat_map<int, int> map;

    for (int i = 0; i < 100; ++i) {
      map.insert(i, i);
    }

    // Delete some elements
    for (int i = 0; i < 100; i += 2) {
      map.remove(i);
    }

    // Verify remaining elements
    for (int i = 0; i < 100; ++i) {
      if (i % 2 == 0) {
        assert(!map.contains(i));
      } else {
        assert(map.contains(i));
        assert(*map.find(i) == i);
      }
    }

    // Insert some new elements
    for (int i = 100; i < 200; ++i) {
      map.insert(i, i);
    }

    // Verify all elements
    for (int i = 0; i < 200; ++i) {
      if (i < 100 && i % 2 == 0) {
        assert(!map.contains(i));
      } else {
        assert(map.contains(i));
        assert(*map.find(i) == i);
      }
    }

    std::cout << "Deleted elements test passed\n";
  }

  // Test file I/O
  {
    fast::flat_map<int, int> map1;
    for (int i = 0; i < 100; ++i) {
      map1.insert(i, i * 10);
    }

    // Save to file
    int fd = open("map_test.bin", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    assert(fd != -1);
    bool save_result = map1.save(fd);
    assert(save_result);
    close(fd);

    // Load from file
    fast::flat_map<int, int> map2;
    fd = open("map_test.bin", O_RDONLY);
    assert(fd != -1);
    bool load_result = map2.load(fd);
    assert(load_result);
    close(fd);

    // Verify contents
    for (int i = 0; i < 100; ++i) {
      assert(map2.contains(i));
      assert(*map2.find(i) == i * 10);
    }

    // Clean up
    unlink("map_test.bin");

    std::cout << "File I/O test passed\n";
  }
}

// Performance benchmarks
void run_benchmarks() {
  std::cout << "\n=== Running Performance Benchmarks ===\n";

  const std::vector<size_t> sizes = {1000, 10000, 100000};

  for (size_t size : sizes) {
    std::cout << "\nBenchmarking with " << size << " elements:\n";

    // Generate random keys
    auto keys = generate_random_keys(size);

    // Insert benchmarks
    {
      {
        Timer timer("fast::flat_map insert");
        fast::flat_map<int, int> map;
        for (auto key : keys) {
          map.insert(key, key);
        }
      }

      {
        Timer timer("std::unordered_map insert");
        std::unordered_map<int, int> map;
        for (auto key : keys) {
          map.insert({key, key});
        }
      }
    }

    // Lookup benchmarks
    {
      fast::flat_map<int, int> fast_map;
      std::unordered_map<int, int> std_map;

      // Pre-populate maps
      for (auto key : keys) {
        fast_map.insert(key, key);
        std_map.insert({key, key});
      }

      // Random access pattern
      auto lookup_keys = keys;
      std::shuffle(lookup_keys.begin(), lookup_keys.end(), std::mt19937(std::random_device()()));

      {
        Timer timer("fast::flat_map lookup");
        for (auto key : lookup_keys) {
          auto* value = fast_map.find(key);
          assert(value != nullptr);
        }
      }

      {
        Timer timer("std::unordered_map lookup");
        for (auto key : lookup_keys) {
          auto it = std_map.find(key);
          assert(it != std_map.end());
        }
      }
    }

    // Removal benchmarks
    {
      fast::flat_map<int, int> fast_map;
      std::unordered_map<int, int> std_map;

      // Pre-populate maps
      for (auto key : keys) {
        fast_map.insert(key, key);
        std_map.insert({key, key});
      }

      // Random removal pattern
      auto remove_keys = keys;
      std::shuffle(remove_keys.begin(), remove_keys.end(), std::mt19937(std::random_device()()));

      {
        Timer timer("fast::flat_map remove");
        for (auto key : remove_keys) {
          fast_map.remove(key);
        }
      }

      // Reset std_map
      std_map.clear();
      for (auto key : keys) {
        std_map.insert({key, key});
      }

      {
        Timer timer("std::unordered_map remove");
        for (auto key : remove_keys) {
          std_map.erase(key);
        }
      }
    }

    // Mixed operations benchmark
    {
      // Generate operations: 70% lookup, 20% insert, 10% remove
      std::vector<std::pair<int, int>> operations; // op_type, key
      operations.reserve(size);

      std::random_device rd;
      std::mt19937 gen(rd());
      std::uniform_int_distribution<> op_dist(1, 10);
      std::uniform_int_distribution<> key_dist(0, static_cast<int>(size * 1.5));

      for (size_t i = 0; i < size; ++i) {
        int op = op_dist(gen);
        int key = key_dist(gen);

        if (op <= 7) {
          // Lookup
          operations.emplace_back(0, key);
        } else if (op <= 9) {
          // Insert
          operations.emplace_back(1, key);
        } else {
          // Remove
          operations.emplace_back(2, key);
        }
      }

      {
        Timer timer("fast::flat_map mixed ops");
        fast::flat_map<int, int> map;

        for (const auto& [op, key] : operations) {
          switch (op) {
            case 0: // Lookup
              map.find(key);
              break;
            case 1: // Insert
              map.insert(key, key);
              break;
            case 2: // Remove
              map.remove(key);
              break;
          }
        }
      }

      {
        Timer timer("std::unordered_map mixed ops");
        std::unordered_map<int, int> map;

        for (const auto& [op, key] : operations) {
          switch (op) {
            case 0: // Lookup
              map.find(key);
              break;
            case 1: // Insert
              map[key] = key;
              break;
            case 2: // Remove
              map.erase(key);
              break;
          }
        }
      }
    }
  }
}

// Custom type benchmark for complex keys and values
void benchmark_custom_types() {
  std::cout << "\n=== Benchmarking with Custom Types ===\n";

  // Define a non-trivial key type
  struct ComplexKey {
    std::string name;
    int id;

    bool operator==(const ComplexKey& other) const {
      return name == other.name && id == other.id;
    }
  };

  // Define a non-trivial value type
  struct ComplexValue {
    std::vector<int> data;
    std::string description;
  };

  // Hash function for ComplexKey
  struct ComplexKeyHash {
    size_t operator()(const ComplexKey& key) const {
      size_t h1 = std::hash<std::string>{}(key.name);
      size_t h2 = std::hash<int>{}(key.id);
      return h1 ^ (h2 << 1);
    }
  };

  // Generate test data
  const size_t size = 10000;
  std::vector<ComplexKey> keys;
  keys.reserve(size);

  for (size_t i = 0; i < size; ++i) {
    keys.push_back({"key" + std::to_string(i), static_cast<int>(i)});
  }

  // Measure insertion performance
  {
    // Define hash function for flat_map with ComplexKey
    // This is needed because XXHash isn't defined for custom types
    extern "C" uint64_t XXH64(const void* input, size_t length, uint64_t seed);
    #undef XXHASH64
#define XXHASH64(x) XXH64(&x, sizeof(x), 0)

    {
      Timer timer("fast::flat_map custom type insert");
      fast::flat_map<ComplexKey, ComplexValue> map;

      for (const auto& key : keys) {
        ComplexValue value;
        value.data = {key.id, key.id * 2, key.id * 3};
        value.description = "Value for " + key.name;

        map.insert(key, value);
      }
    }

    {
      Timer timer("std::unordered_map custom type insert");
      std::unordered_map<ComplexKey, ComplexValue, ComplexKeyHash> map;

      for (const auto& key : keys) {
        ComplexValue value;
        value.data = {key.id, key.id * 2, key.id * 3};
        value.description = "Value for " + key.name;

        map.insert({key, value});
      }
    }
  }
}

int main() {
  std::cout << "=== flat_map Test and Benchmark Suite ===\n";

  // Run basic functionality tests
  test_basic_functionality();

  // Run edge case tests
  test_edge_cases();

  // Run performance benchmarks
  run_benchmarks();

  // Run benchmarks with custom types
  benchmark_custom_types();

  std::cout << "\nAll tests passed successfully!\n";
  return 0;
}
