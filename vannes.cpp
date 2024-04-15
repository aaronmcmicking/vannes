#include <iostream>
#include "core/RAM.cpp"
#include "core/CPU.cpp"
#include "common/log.hpp"
#include "common/util.hpp"
#include "common/nes_assert.hpp"
#include "cartridge/cartridge.cpp"

int main(){
    using namespace VNES_LOG;
    std::cout << "Hello World!" << std::endl;

//    if(!using_sign_2_comp()){
//        LOG(Severity::WARN, "Detected non-sign 2's complemented arithmetic! Sign 2's complement is required. Check the compiler or platform implementation.");
//    }

    //VNES_LOG::log_level = VNES_LOG::ERROR;

    RAM ram = RAM();
    uint16_t ptr = 0x0000;
    ram.write_reset_vec(ptr);
    ram.write(ptr++, CPU::SEC_IMPL);
    ram.write(ptr++, CPU::CLC_IMPL);
    ram.write(ptr++, CPU::NOP_IMPL);
    PPU ppu = PPU();
    CPU cpu = CPU(ram, ppu);
    cpu.set_status_reg(0);
    for(int i = 0; i < ptr; i++){
        std::cout << binary_string(cpu.status_as_int(), 8) << std::endl;
        cpu.step();
    }

    Cartridge cart = Cartridge("roms/Super Mario Bros. (Japan, USA).nes");
    //cart.dump_rom();

    return 0;
}
