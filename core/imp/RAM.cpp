#pragma once

#include "../RAM.hpp"
#include "../../common/nes_assert.hpp"

void RAM::write(uint16_t addr, uint8_t data){
    VNES_ASSERT(addr <= 0xFFFF);
    mem[addr] = data;
}

uint8_t RAM::read(uint16_t addr){
    VNES_ASSERT(addr <= 0xFFFF);
    return mem[addr];
}
