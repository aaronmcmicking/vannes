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
        Cartridge(); // dummy cart with 16k PRG and 8k CHR available
        Cartridge(std::string filename);
        void load_rom(std::string filename);
        void load_dummy_rom();
        void dump_rom();

        uint8_t read(uint16_t addr); // reads from mapper
        void    write(uint16_t addr, uint8_t data); // write to cart RAM, sometimes battery backed 

        uint8_t read_pallete(uint16_t addr);
        void write_pallete(uint16_t addr, uint8_t data);

        typedef enum NametableLayout{
            VERTICAL = 0, // vertical arrangement = "horizontally mirrored"
            HORIZONTAL = 1 // horizontally arrangement = "verically mirrored"
        }NametableLayout;

        NametableLayout nametable_layout;

    private:
        std::unique_ptr<Mapper> mapper; 
        void set_mapper();

        // see  for explanations of flags

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
        //
        void parse_iNES2_header();
        void parse_iNES_header();
};
