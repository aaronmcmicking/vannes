#include <iostream>
#include "core/imp/RAM.cpp"
#include "core/imp/CPU.cpp"
#include "common/log.hpp"

int main(){
    std::cout << "Hello World!" << std::endl;

    RAM ram = RAM();
    CPU cpu = CPU(ram);
    (void)cpu;

    return 0;
}
