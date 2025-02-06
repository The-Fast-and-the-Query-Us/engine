#pragma once

namespace fast {

template <class T>
void swap(T &lhs, T &rhs) {
  T tmp = lhs;
  lhs = rhs;
  rhs = tmp;
}

}
