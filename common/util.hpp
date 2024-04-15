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

std::string binary_string(uint64_t num, int width){
    // cast all to uint64, then onyl print out width (allows any int width to be passed without overloading) 

    std::string bin {};
    for(int i = 0; i < width; i++){
        bin += (num >> i) & 1 ? '1' : '0';
    }
    std::reverse(bin.begin(), bin.end());
    return bin;
}
