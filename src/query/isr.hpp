#pragma once

#include <hashtable.hpp>
#include <post_list.hpp>
#include <isr.hpp>

namespace fast::query {

// add phrase, OR, and AND
class isr_or : public isr {

};

class isr_and : public isr {

};

class isr_phrase : public isr {

};

}
