#pragma once

#include "string.hpp"

namespace fast::english {

void trim_puntuation(fast::string &word);

void porter_stem(fast::string &word);

bool is_stop_word(const fast::string &word);

}
