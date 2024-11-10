#pragma once

// see https://9bitworkshop.com/blog/platforms/nintendo-nes.md.html

#include <stdint.h>
#include "../../cartridge/cartridge.hpp"
#include "../../controllers/Controller.hpp"

class RAM{
    public:
        RAM(Cartridge& cart, Controller& controller);

        uint8_t read(uint16_t addr);
        void    write(uint16_t addr, uint8_t data);

        void dump(); // dumps RAM contents (as visible through RAM::read() calls) to stdout

        // The addresses of the the reserved 16 bit vectors, in little
        // endian format. The lower 8 bits are stored at ADDR and the higher at
        // ADDR+1
        enum VEC_ADDR : uint16_t{
            NMI_VEC = 0xFFFA,
            RESET_VEC = 0xFFFC,
            BRK_VEC = 0xFFFE
        };

    private:
        Cartridge& cart;
        Controller& controller;

        /*
         * 0x0000 - 0x00FF: RAM (zero-page)
         * 0x0100 - 0x01FF: RAM (CPU stack)
         * 0x0200 - 0x07FF: RAM (general-purpose)
         * 0x0800 - 0x1FFF: Mirror of General RAM
         * 0x2000 - 0x2007: PPU registers
         * 0x2008 - 0x3FFF: Mirror of PPU registers
         * 0x4000 - 0x400F: APU registers
         * 0x4010 - 0x4017: DMC, joystick, APU registers
         * 0x4018 - 0x401A: Unused (scrapped functionality), sometimes testing registers (disabled in real use)
         * 0x4020 - 0x5FFF: Cartridge (maybe mapper registers)
         * 0x6000 - 0x7FFF: Cartridge RAM (may be bank-switched, battery-backed)
         * 0x8000 - 0xFFFF: Cartridge ROM (may be bank-switched)
         * 0xFFFA - 0xFFFB: NMI Vector
         * 0xFFFC - 0xFFFD: Reset Vector
         * 0xFFFE - 0xFFFF: BRK Vector
         */
        static const int INTERNAL_MEM_ITEMS = 0x4020;
        uint8_t mem[INTERNAL_MEM_ITEMS];
};
