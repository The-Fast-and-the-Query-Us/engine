#pragma once

#include <string.hpp>

namespace fast::english {

// Todo
void trim_puntuation(fast::string &word);

void porter_stem(fast::string &word);

// Todo
bool is_stop_word(const fast::string &word);

}
