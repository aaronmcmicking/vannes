#include <iostream>
#include "core/RAM.cpp"
#include "core/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"
#include "cartridge/cartridge.cpp"
#include "mappers/Mapper000.cpp"

int main(){
    using namespace VNES_LOG;

    Cartridge cart = Cartridge("roms/Super Mario Bros. (Japan, USA).nes");
    RAM ram = RAM(cart);
    PPU ppu = PPU();
    CPU cpu = CPU(ram, ppu);

    int i = 0;
    for(i = 0; i < 100; i++){
        cpu.step();
    }

    return 0;
}
