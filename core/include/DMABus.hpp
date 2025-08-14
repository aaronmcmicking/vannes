#pragma once

#include "RAM.hpp"

/*
 * Allows PPU to read directly from RAM
 */
class DMABus{
    public:
        DMABus(RAM& _ram);

        uint8_t read(uint16_t addr);

    private:
        RAM& ram;
};
