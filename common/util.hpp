#pragma once

#include "log.hpp"
#include <limits.h>
#include <string>
#include <algorithm>

// not necessary, but kinda interesting?
#pragma GCC push_options
#pragma GCC optimize ("O0")
bool using_sign_2_comp(){
    return INT_MIN - 1 >= INT_MAX;
}
#pragma GCC pop_options
