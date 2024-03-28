#pragma once

#include "../RAM.hpp"
#include "../../common/nes_assert.hpp"
#include "../../common/log.hpp"

RAM::RAM(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing RAM...");
}

void RAM::write(uint16_t addr, uint8_t data){
    mem[addr] = data;
}

uint8_t RAM::read(uint16_t addr){
    return mem[addr];
}
