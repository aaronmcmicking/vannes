#include <iostream>
#include "core/RAM.cpp"
#include "core/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"
#include "cartridge/cartridge.cpp"
#include "mappers/Mapper000.cpp"
#include <chrono>

void parse_args(int argc, char** argv, std::string& rom_filename){
    for(int i = 1; i < argc; i++){
        std::string arg {argv[i]};
        int split_pos = arg.find("=");
        std::string variable {arg.substr(0, split_pos)};
        std::string value {arg.substr(split_pos+1)}; 
        //std::cout << "found: " << variable << " = " << value << std::endl;

        if(variable == "rom"){
            rom_filename = value;
        }else if(variable == "log_level"){
            VNES_LOG::log_level = (VNES_LOG::Severity)std::atoi(value.c_str());
        }else if(variable == "log_to_file"){
            VNES_LOG::file_out = (value == "1");
            //if(value == "1"){ VNES_LOG::file_out = true; }
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


    std::string rom_filename {"roms/Super Mario Bros. (Japan, USA).nes"};
    parse_args(argc, argv, rom_filename);
    init_log();

    Cartridge cart = Cartridge(rom_filename);
    RAM ram = RAM(cart);
    //ram.write(PPU::PPU_STATUS, 0xFF); // programs wait for PPU at reset
    PPU ppu = PPU(ram);
    CPU cpu = CPU(ram, ppu);

    int i = 0;
    std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();
    int steps_to_do = 100;
    for(i = 0; i < steps_to_do; i++){
        cpu.step();
    }
    std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
    std::cout << steps_to_do << " steps took " << std::chrono::duration_cast<std::chrono::seconds> (end - begin).count() << "s" << std::endl;
    std::cout << steps_to_do << " steps took " << std::chrono::duration_cast<std::chrono::milliseconds> (end - begin).count() << "ms" << std::endl;
    std::cout << steps_to_do << " steps took " << std::chrono::duration_cast<std::chrono::microseconds> (end - begin).count() << "µs" << std::endl;
    printf("(cpu did %ld cycles since reset)\n", cpu.cycles_since_reset);

    //for(int i = 0; i < 10; i++) { ram.write(i, i); ram.write(i+0x8000, i); }
    //ram.dump();
    //LOG(DEBUG, "\nPPU cycles_since_reset: %lld\nPPU frame_cycles: %d\nPPU scanline_cycles: %d\nPPU scanlines: %d", ppu.cycles_since_reset, ppu.frame_cycles, ppu.scanline_cycles, ppu.scanlines);

    cpu.reset();
    //LOG(DEBUG, "\nPPU cycles_since_reset: %lld\nPPU frame_cycles: %d\nPPU scanline_cycles: %d\nPPU scanlines: %d", ppu.cycles_since_reset, ppu.frame_cycles, ppu.scanline_cycles, ppu.scanlines);

    return 0;
}

