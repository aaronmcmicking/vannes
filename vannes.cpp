#include <iostream>
#include "core/RAM.cpp"
#include "core/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"
#include "cartridge/cartridge.cpp"
#include "mappers/Mapper000.cpp"

void parse_args(int argc, char** argv, std::string& rom_filename, VNES_LOG::Severity& log_level){
    for(int i = 1; i < argc; i++){
        std::string arg {argv[i]};
        int split_pos = arg.find("=");
        std::string variable {arg.substr(0, split_pos)};
        std::string value {arg.substr(split_pos+1)}; 
        //std::cout << "found: " << variable << " = " << value << std::endl;

        if(variable == "rom"){
            rom_filename = value;
        }else if(variable == "log_level"){
            log_level = (VNES_LOG::Severity)std::atoi(value.c_str());
        }else{
            VNES_LOG::LOG(VNES_LOG::WARN, "Unknown argument '%s'", variable.c_str());
            VNES_ASSERT(0 && "Bad argument");
        }
    }
    //std::cout << "set: " << rom_filename << std::endl;
    //std::cout << "set: " << log_level << std::endl;
}

int main(int argc, char** argv){
    using namespace VNES_LOG;

    init_log();

    std::string rom_filename {"roms/Super Mario Bros. (Japan, USA).nes"};
    parse_args(argc, argv, rom_filename, VNES_LOG::log_level);

    Cartridge cart = Cartridge(rom_filename);
    RAM ram = RAM(cart);
    ram.write(PPU::PPU_STATUS, 0xFF); // programs wait for PPU at reset
    PPU ppu = PPU();
    CPU cpu = CPU(ram, ppu);

    int i = 0;
    for(i = 0; i < 10; i++){
        cpu.step();
    }

    //for(int i = 0; i < 10; i++) { ram.write(i, i); ram.write(i+0x8000, i); }
    ram.dump();

    return 0;
}
