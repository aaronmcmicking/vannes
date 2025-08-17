#pragma once

#include <string>
#include <vector>
#include <optional>
#include <array>
#include "../mappers/Mapper.hpp"
#include <memory>
#include "header.cpp"

/*
 * Follows iNES and NES2.0 standards, but does not implement all features. 
 * Ignores PlayChoice data.
 */
class Cartridge{
    public:
        //Cartridge(); // dummy cart with 16k PRG and 8k CHR available
        Cartridge(std::string filename);
        //void load_rom_old(std::string filename);
        void load_rom(std::string filename);
        void dump_rom();

        uint8_t read(uint16_t addr); // reads from mapper
        void    write(uint16_t addr, uint8_t data); // write to cart RAM, sometimes battery backed 

        uint8_t read_pallete(uint16_t addr);

        typedef enum NametableLayout{
            VERTICAL = 0,
            HORIZONTAL = 1
        }NametableLayout;

        NametableLayout nametable_layout {};

    private:
        std::unique_ptr<Mapper> mapper; 
        void set_mapper();

        // see https://www.nesdev.org/wiki/INES for explanations of flags
        // and other header fields
        struct iNESHeader{
            uint8_t nes_title[4];       // -usually 'NES' + DOS null terminator
            uint8_t prg_rom_size_x16kb; // -size of PRG ROM in 16kb chunks
            uint8_t chr_rom_size_x8kb;  // -size of CHR ROM in 8kb chunks
            uint8_t flags_6;            // -Mapper low nybble, alt nametable, trainer present, nametable arrangement
            uint8_t flags_7;            // -Mapper high nybble, if bits[3:2] = 2 then remaining are NES2.0 spec, PlayChoice 10 = unused, VS mode = coinslot
            uint8_t flags_8;            // -PRG RAM size, in 8kb chunks. Very rarely used due to being new spec addition
            uint8_t flags_9;            // -bits[7:1] = unused, bits[0]: TV system (0: NTSC; 1: PAL), rarely honoured
            uint8_t padding[6];         // -6 bytes of padding succeed the header. 
                                        //  There is technically flag 10, but it isn't part of the spec. From NESDEV.org:
                                        //  "This byte is not part of the official specification, and relatively few emulators honor it.
                                        //  The PRG RAM Size value (stored in byte 8) was recently added to the official specification; 
                                        //  as such, virtually no ROM images in circulation make use of it. Older versions of the 
                                        //  iNES emulator ignored bytes 7-15, and several ROM management tools wrote messages in 
                                        //  there. Commonly, these will be filled with "DiskDude!", which results in 64 being added 
                                        //  to the mapper number. A general rule of thumb: if the last 4 bytes are not all zero, and 
                                        //  the header is not marked for NES 2.0 format, an emulator should either mask off the upper 
                                        //  4 bits of the mapper number or simply refuse to load the ROM."
        };

        struct iNES2Header{
            uint8_t nes_title[4];       // -usually 'NES' + DOS null terminator
            uint8_t prg_rom_size_lsb;   // -lsb of PRG ROM size
            uint8_t chr_rom_size_lsb;   // -lsb of CHR ROM size
            uint8_t flags_6;            // -Mapper low nybble, alt nametable, trainer present, nametable arrangement
            uint8_t flags_7;            // -Mapper high nybble, if bits[3:2] = 2 then remaining are NES2.0 spec, PlayChoice 10 = unused, VS mode = coinslot
            uint8_t mapper_msb_submapper;            // -PRG RAM size, in 8kb chunks. Very rarely used due to being new spec addition
            uint8_t prg_chr_rom_msb;            // -bits[7:1] = unused, bits[0]: TV system (0: NTSC; 1: PAL), rarely honoured
            uint8_t prg_ram_eeprom_sift;         // -6 bytes of padding succeed the header. 
            uint8_t chr_ram_size;
            uint8_t cpu_ppu_timing;
            uint8_t hardware_type;
            uint8_t misc_roms_present;  
            uint8_t default_expansion_device;

        };

        Header header;
        uint16_t mapper_number = 0;
        uint8_t submapper_number = 0;
        std::optional<std::array<uint8_t, 512>> trainer;

        uint32_t prg_rom_size_bytes = 0;
        uint32_t prg_ram_size_bytes = 0;
        uint32_t eeprom_size_bytes = 0;

        uint32_t chr_rom_size_bytes = 0;
        uint32_t chr_ram_size_bytes = 0;
        uint32_t chr_nvram_size_bytes = 0;

        std::vector<uint8_t> prg_rom; // loaded in entirety, mapper is responsible for accessing properly
        std::vector<uint8_t> chr_rom; // loaded in entirety, mapper is responsible for accessing properly
        // Mapper should handle banked ROM data
};
