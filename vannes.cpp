#include <iostream>
#include "core/RAM.cpp"
#include "core/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"
#include "cartridge/cartridge.cpp"
#include "mappers/Mapper000.cpp"

int main(int argc, char** argv){
    using namespace VNES_LOG;

    //std::string rom_filename {"roms/Super Mario Bros. (Japan, USA).nes"};
    std::string rom_filename {"roms/Super Mario Bros. (Japan, USA).nes"};
    if(argc > 1){
        rom_filename = argv[1];
    }
    if(argc > 2){
        VNES_LOG::log_level = (VNES_LOG::Severity)std::atoi((argv[2]));
    }

    Cartridge cart = Cartridge(rom_filename);
    RAM ram = RAM(cart);
    ram.write(PPU::PPU_STATUS, 0xFF); // programs wait for PPU at reset
    PPU ppu = PPU();
    CPU cpu = CPU(ram, ppu);

    int i = 0;
    for(i = 0; i < 100; i++){
        cpu.step();
    }

    return 0;
}
