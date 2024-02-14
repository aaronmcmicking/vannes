#include <iostream>
#include "core/imp/RAM.cpp"
#include "core/imp/CPU.cpp"

int main(){
    std::cout << "Hello World!" << std::endl;

    RAM ram = RAM();

    for(int i = 0; i < 0x07FF; i++){
        ram.write(i, i*2);
    }

    std::cout << std::hex << std::uppercase;
    for(int i = 0; i < 0x07FF; i++){
        //printf("0x%02x\n", ram.read(i));
        std::cout << ram.read(i) << std::endl;
    }
    std::cout << std::nouppercase << std::dec << std::endl;
    
    unsigned long c = 0;
    for(int i = 0; i < 256; i++){
        c += i;
    }

    return 0;
}
