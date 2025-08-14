#pragma once

#include "include/DMABus.hpp"

DMABus::DMABus(RAM& _ram): ram {_ram} { }

uint8_t DMABus::read(uint16_t addr){
    return ram.read(addr);
}
