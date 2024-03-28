#include <iostream>
#include "core/imp/RAM.cpp"
#include "core/imp/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"

int main(){
    using namespace VNES_LOG;
    std::cout << "Hello World!" << std::endl;

    if(!using_sign_2_comp()){
        LOG(Severity::WARN, "Detected non-sign 2's complemented arithmetic! Sign 2's complement is required. Check the compiler or platform implementation.");
    }

    RAM ram = RAM();
    CPU cpu = CPU(ram);
    (void)cpu;

    return 0;
}
