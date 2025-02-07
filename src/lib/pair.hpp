namespace fast {

template <typename T1, typename T2>
class pair {
public:
  T1 first;
  T2 second;

  // constructor
  pair(T1 f, T2 s) : first(f), second(s) {}

  // copy constructor
  pair(pair<T1, T2> *other) {
    first = other->first;
    second = other->second;
  }

  // move constructor
  pair(pair<T1, T2> &&other) {
    first = other.first;
    second = other.second;
  }

  // assignment operator
  pair operator=(pair<T1, T2> &other) {
    if (this != &other) {
      first = other.first;
      second = other.second;
    }

    return *this;
  }
};

} // namespace fast
