#pragma once

#include "include/RAM.hpp"
#include "../common/nes_assert.hpp"
#include "../common/log.hpp"
#include <stdlib.h>

RAM::RAM(Cartridge& cart): cart {cart} {
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing RAM...");
    // init RAM to 0, although this shouldn't be relied upon as actual reset
    // values were undefined (and unpredictable) in real hardware
    for(int i = 0; i < INTERNAL_MEM_ITEMS; i++){
        mem[i] = 0;
    }
}

void RAM::write(uint16_t addr, uint8_t data){
    switch(addr){
        case 0x0000 ... 0x7FFF:
            mem[addr] = data;
            break;
        case 0x8000 ... 0xFFFF:
            VNES_LOG::LOG(VNES_LOG::ERROR, "RAM.write(): Cannot write to read-only ROM address %x");
            break;
        default:
            VNES_LOG::LOG(VNES_LOG::FATAL, "RAM.write(): Bad address %x could not be mapped to mapper or internal RAM! How is this possible??");
            exit(1);
            break;
    }
}

uint8_t RAM::read(uint16_t addr){
    using namespace VNES_LOG;
    switch(addr){
        case 0x0000 ... 0x401F:
            LOG(DEBUG, "Read value %x from address %x (internal RAM)", mem[addr], addr);
            return mem[addr];
            break;
        case 0x4020 ... 0xFFFF:
            LOG(DEBUG, "Read value %x from address %x (cartridge)", cart.read(addr), addr);
            return cart.read(addr);
            break;
        default:
            LOG(FATAL, "RAM.read(): Bad address %x could not be mapped to mapper or internal RAM! How is this possible??");
            exit(1);
            break;
    }
}
