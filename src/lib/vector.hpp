// vector.h
//
// Starter file for a vector template

#pragma once

#include <cassert>
#include <cstddef> // for size_t
#include <cstdlib>

namespace fast {

template <typename T>
class vector {
public:
  // Default Constructor
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Constructs an empty vector with capacity 0
  vector() : elts{nullptr}, cap_{0}, size_{0} {}

  // Destructor
  // REQUIRES: Nothing
  // MODIFIES: Destroys *this
  // EFFECTS: Performs any neccessary clean up operations
  ~vector() { delete[] elts; }

  // Resize Constructor
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Constructs a vector with size num_elements,
  //    all default constructed
  vector(size_t num_elements)
      : elts{new T[num_elements]}, size_{num_elements}, cap_{num_elements} {}

  // Fill Constructor
  // REQUIRES: Capacity > 0
  // MODIFIES: *this
  // EFFECTS: Creates a vector with size num_elements, all assigned to val
  vector(size_t num_elements, const T &val)
      : elts{new T[num_elements]}, size_{num_elements}, cap_{num_elements} {
    for (size_t i = 0; i < num_elements; ++i) {
      elts[i] = val;
    }
  }

  // Copy Constructor
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Creates a clone of the vector other
  vector(const vector<T> &other) {
    size_ = other.size_;
    cap_ = other.cap_;
    elts = new T[cap_];

    for (size_t i = 0; i < size_; ++i) {
      elts[i] = other.elts[i];
    }
  }

  // Assignment operator
  // REQUIRES: Nothing
  // MODIFIES: *this
  // EFFECTS: Duplicates the state of other to *this
  vector operator=(const vector<T> &other) {
    delete[] elts;

    cap_ = other.cap_;
    size_ = other.size_;
    elts = new T[cap_];

    for (size_t i = 0; i < size_; ++i) {
      elts[i] = other.elts[i];
    }
    return *this;
  }

  // Move Constructor
  // REQUIRES: Nothing
  // MODIFIES: *this, leaves other in a default constructed state
  // EFFECTS: Takes the data from other into a newly constructed vector
  vector(vector<T> &&other) {
    elts = other.elts;
    cap_ = other.cap_;
    size_ = other.size_;

    other.elts = nullptr;
    other.cap_ = 0;
    other.size_ = 0;
  }

  // Move Assignment Operator
  // REQUIRES: Nothing
  // MODIFIES: *this, leaves otherin a default constructed state
  // EFFECTS: Takes the data from other in constant time
  vector operator=(vector<T> &&other) {
    delete[] elts;

    elts = other.elts;
    cap_ = other.cap_;
    size_ = other.size_;

    other.elts = nullptr;
    other.cap_ = 0;
    other.size_ = 0;

    return *this;
  }

  // REQUIRES: new_capacity > capacity( )
  // MODIFIES: capacity( )
  // EFFECTS: Ensures that the vector can contain size( ) = new_capacity
  //    elements before having to reallocate
  void reserve(size_t newCapacity) { grow(newCapacity); }

  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns the number of elements in the vector
  size_t size() const { return size_; }

  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns the maximum size the vector can attain before resizing
  size_t capacity() const { return cap_; }

  // REQUIRES: 0 <= i < size( )
  // MODIFIES: Allows modification of data[i]
  // EFFECTS: Returns a mutable reference to the i'th element
  T &operator[](size_t i) { return elts[i]; }

  // REQUIRES: 0 <= i < size( )
  // MODIFIES: Nothing
  // EFFECTS: Get a const reference to the ith element
  const T &operator[](size_t i) const { return elts[i]; }

  // REQUIRES: Nothing
  // MODIFIES: this, size( ), capacity( )
  // EFFECTS: Appends the element x to the vector, allocating
  //    additional space if neccesary
  void pushBack(const T &x) {
    if (size_ == cap_) {
      const auto new_cap = (8 > cap_ << 1) ? 8 : cap_ << 1;
      grow(new_cap);
    }
    elts[size_++] = x;
  }

  // REQUIRES: Nothing
  // MODIFIES: this, size( )
  // EFFECTS: Removes the last element of the vector,
  //    leaving capacity unchanged
  void popBack() {
    if (size_ > 0)
      elts[--size_].~T();
  }

  // REQUIRES: Nothing
  // MODIFIES: Allows mutable access to the vector's contents
  // EFFECTS: Returns a mutable random access iterator to the
  //    first element of the vector
  T *begin() { return elts; }

  // REQUIRES: Nothing
  // MODIFIES: Allows mutable access to the vector's contents
  // EFFECTS: Returns a mutable random access iterator to
  //    one past the last valid element of the vector
  T *end() { return elts + size_; }

  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns a random access iterator to the first element of the
  // vector
  const T *begin() const { return elts; }

  // REQUIRES: Nothing
  // MODIFIES: Nothing
  // EFFECTS: Returns a random access iterator to
  //    one past the last valid element of the vector
  const T *end() const { return elts + size_; }

private:
  T *elts;
  size_t size_;
  size_t cap_;

  void grow(size_t new_cap) {
    const auto new_elts = new T[new_cap];
    for (size_t i = 0; i < size_; ++i) {
      new_elts[i] = elts[i];
    }
    delete[] elts;
    elts = new_elts;
    cap_ = new_cap;
  }
};
} // namespace fast
