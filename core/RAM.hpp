#pragma once

// see https://9bitworkshop.com/blog/platforms/nintendo-nes.md.html

#include <stdint.h>

class RAM{
    public:
        RAM();
        uint8_t read(uint16_t addr);
        void write(uint16_t addr, uint8_t data);

        // The first address of the the reserved 16 bit vectors.
        // The lower 8 bits are stored at ADDR and the higher at
        // ADDR+1
        enum VEC_ADDR : uint16_t{
            NMI_VEC = 0xFFFA,
            RESET_VEC = 0xFFFC,
            BRK_VEC = 0xFFFE
        };

    private:
        /*
         * 0x0000 - 0x00FF: RAM (zero-page)
         * 0x0100 - 0x01FF: RAM (CPU stack)
         * 0x0200 - 0x07FF: RAM (general-purpose)
         * 0x0800 - 0x1FFF: Mirror of General RAM
         * 0x2000 - 0x2007: PPU registers
         * 0x2008 - 0x3FFF: Mirror of PPU registers
         * 0x4000 - 0x400F: APU registers
         * 0x4010 - 0x4017: DMC, joystick, APU registers
         * 0x4020 - 0x5FFF: Cartridge (maybe mapper registers)
         * 0x6000 - 0x7FFF: Cartridge RAM (maybe battery-backed)
         * 0xFFFA - 0xFFFB: NMI Vector
         * 0xFFFC - 0xFFFD: Reset Vector
         * 0xFFFE - 0xFFFF: BRK Vector
         * 0x8000 - 0xFFFF: PRG ROM (maybe bank-switched)
         */
        uint8_t mem[65536];
};
