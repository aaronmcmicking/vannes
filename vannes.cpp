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
    std::cout << "Hello World!" << std::endl;

//    if(!using_sign_2_comp()){
//        LOG(Severity::WARN, "Detected non-sign 2's complemented arithmetic! Sign 2's complement is required. Check the compiler or platform implementation.");
//    }

    //VNES_LOG::log_level = VNES_LOG::ERROR;

    Cartridge cart = Cartridge("roms/Super Mario Bros. (Japan, USA).nes");
    RAM ram = RAM(cart);
    PPU ppu = PPU();
    CPU cpu = CPU(ram, ppu);

    int i = 0;
    for(i = 0; i < 100; i++){
        //cpu.set_status_reg(0b10000000);
        cpu.step();
    }
    printf("%d\n", i);

    //cpu.set_status_reg(0);
    //for(int i = 0; i < ptr; i++){
    //    std::cout << binary_string(cpu.status_as_int(), 8) << std::endl;
    //    cpu.step();
    //}

    //cart.dump_rom();


    Mapper map {};
    std::cout << map.name << std::endl;
    

    return 0;
}
