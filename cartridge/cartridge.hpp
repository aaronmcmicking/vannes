#pragma once

#include <string>
#include <vector>
#include <optional>
#include <array>
#include "../mappers/Mapper.hpp"
#include <memory>

/*
 * Follows iNES and NES2.0 standards, but does not implement all features. 
 * Ignores PlayChoice data.
 */
class Cartridge{
    public:
        Cartridge(std::string filename);
        void load_rom(std::string filename);
        void dump_rom();

        uint8_t read(uint16_t addr); // reads from mapper

    private:
        std::unique_ptr<Mapper> mapper; // ptr to smooth out initialization weirdness
        void set_mapper();
        std::unique_ptr<Mapper> get_default_mapper();

        // see https://www.nesdev.org/wiki/INES for explanations of flags
        // and other header fields
        struct Header{
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

        Header header;
        int mapper_number;
        std::optional<std::array<uint8_t, 512>> trainer;
        std::vector<uint8_t> prg_rom; // loaded in entirety, mapper is responsible for accessing properly
        std::vector<uint8_t> chr_rom; // loaded in entirety, mapper is responsible for accessing properly
        // Mapper should handle banked ROM data
};
