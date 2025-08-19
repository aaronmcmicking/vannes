#pragma once

#include <inttypes.h>
#include <fstream>

// Data always assumed to be iNES2 format, since iNES2 is backwards compatible
// with iNES. If iNES format is found, bytes 4, 5, 8, 9 are interpreted slightly
// differently, byte 10 is ignored, and bytes 11-15 are 0 (padding)
//
// see https://www.nesdev.org/wiki/INES
// and https://www.nesdev.org/wiki/NES_2.0
struct Header{
    public:
        typedef enum HeaderFormat{
            iNES = 1,
            iNES2
        }HeaderFormat;

        typedef struct Data{
            uint8_t nes_title[4];               // -usually 'NES' + DOS null terminator
            uint8_t prg_rom_size_lsb;           // -PRG ROM size LSB [7:0]
            uint8_t chr_rom_size_lsb;           // -CHR ROM size LSB [7:0]
            uint8_t flags_6;                    // -Mapper low nybble [7:4], alt nametable y/n [3], trainer present [2], battery present [1], nametable arrangement [0]
            uint8_t flags_7;                    // -Mapper middle nybble [7:4], if bits[3:2] = 2 then remaining are NES2.0 spec, console type (0: NES, 1: Vs. system, 2: Playchoice 10, 3: extended)
            uint8_t mapper_msb_submapper;       // -Submapper # [7:4], Mapper high nybble [3:0] 
            uint8_t prg_chr_rom_msb;            // -CHR-ROM size MSB [7:4], PRG-ROM size [3:0]
            uint8_t prg_ram_eeprom_shift;       // -PRG-NVRAM/EEPROM (non-volatile) shift count [7:4], PRG-RAM (volatile) shift count [3:0]
                                                // If the shift count is zero, there is no PRG-(NV)RAM.
                                                // If the shift count is non-zero, the actual size is
                                                // "64 << shift count" bytes, i.e. 8192 bytes for a shift count of 7.
            uint8_t chr_ram_shift;              // -CHR-NVRAM size (non-volatile) shift count [7:4], CHR-RAM (volatile) shift count [3:0]
                                                // Same shift meaning as above
            uint8_t cpu_ppu_timing;             // -[1:0] CPU/PPU timing mode (0: RP2C02 (NTSC), 1: RP2C07 (Licenced PAL  NES), 2: Multiple region, 3: UA6538 (Dendy), [7:2] unused
            uint8_t hardware_type;              // -System type
                                                // If VS. System type (byte 7): Vs. hardware type [7:4], Vs. PPU type [3:0]
                                                // If Extended console (byte 7): [3:0] Extended console type, [7:4] unused
            uint8_t misc_roms_present;          // -[1:0] Number of misc ROMs present, [7:2] unused
                                                // This emulator ignores misc ROM data due to not being used enough
                                                // see https://www.nesdev.org/wiki/NES_2.0#Miscellaneous_ROM_Are/
            uint8_t default_expansion_device;   // -[5:0] Default expansion device, [7:6] unused
        }Data;

        Header();
        Header(std::ifstream& file);

        HeaderFormat header_format;
        Data data;
};
