#pragma once

#include <algorithm>
#include <bitset>
#include <cmath>
#include <exception>
#include <fstream>
#include <ios>
#include <iostream>
#include <memory>
#include "cartridge.hpp"
#include "../common/log.hpp"
#include "../common/nes_assert.hpp"
#include "../mappers/mapper_includes.hpp"

Cartridge::Cartridge(std::string filename): mapper {nullptr} {
    load_rom(filename);
    set_mapper(); 
}

Cartridge::Cartridge(): mapper {nullptr}{ // dummy cart with 16k PRG and 8k CHR available
    using namespace VNES_LOG;

    load_dummy_rom();
    set_mapper();
}

void Cartridge::set_mapper(){
    switch(mapper_number){
        case 0:
            mapper = std::make_unique<Mapper000>(prg_rom, chr_rom);
            break;
        default:
            mapper = std::make_unique<Mapper000>(prg_rom, chr_rom);
            VNES_LOG::LOG(VNES_LOG::WARN, "Mapper number %d not recognized, setting default %s", mapper_number, mapper->name.c_str());
            break;
    }
    VNES_LOG::LOG(VNES_LOG::INFO, "Set cartridge mapper to %s", mapper->name.c_str());
}

void Cartridge::load_dummy_rom(){
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Constructing dummy cartridge");

    header = Header(); // get dummy header

    parse_iNES_header();

    trainer.reset();

    // PRG ROM
    prg_rom.clear();
    prg_rom.resize(prg_rom_size_bytes);
    std::fill(prg_rom.begin(), prg_rom.end(), 1);

    // CHR ROM
    chr_rom.clear();
    chr_rom.resize(chr_rom_size_bytes);
    std::fill(chr_rom.begin(), chr_rom.end(), 2);
    

    VNES_LOG::LOG(VNES_LOG::DEBUG, "Finished constructing dummy cartridge");

}

void Cartridge::load_rom(std::string filename){
    using namespace VNES_LOG;

    LOG(INFO, "Loading ROM.");

    std::ifstream file {};
    file.exceptions(std::iostream::failbit | std::iostream::badbit | std::iostream::eofbit);
    file.open(filename, std::ios::binary | std::ios::in);
    
    if(!file.is_open()){
        LOG(FATAL, "Failed to open ROM file to read. Exiting");
        VNES_ASSERT(0 && "Failed to open ROM file for reading");
    }else{
        LOG(INFO, "Opened ROM file: %s", filename.c_str());
    }

    header = Header(file);

    if(header.header_format == Header::iNES){
        // see https://www.nesdev.org/wiki/INES
        parse_iNES_header(); 
    }else if(header.header_format == Header::iNES2){
        // see https://www.nesdev.org/wiki/NES_2.0
        parse_iNES2_header();
    } 

    LOG(DEBUG, "Finished parsing header");

    // Read trainer, PRG, CHR data in from file
    
    file.seekg(16); // make sure we are at the end of the header
     
    try{
        // trainer
        if(trainer.has_value()){
            // trainer is a 512 byte region immediately after the header
            // and before the PRG/CHR data that should be loaded into address
            // 0x7000 in CPU memory. This is due to some workarounds for different
            // cartridge hardware.

            std::array<uint8_t, 512> trainer_array = trainer.value();
            char* trainer_array_raw = reinterpret_cast<char*>(trainer_array.data());
            file.read(trainer_array_raw, 512);
        }else{
            trainer.reset();
        }

        // PRG ROM
        prg_rom.clear();
        prg_rom.resize(prg_rom_size_bytes);
        char* prg_rom_ptr = reinterpret_cast<char *>(prg_rom.data());
        file.read(prg_rom_ptr, prg_rom_size_bytes);

        // CHR ROM
        chr_rom.clear();
        chr_rom.resize(chr_rom_size_bytes);
        char* chr_rom_ptr = reinterpret_cast<char *>(chr_rom.data());
        file.read(chr_rom_ptr, chr_rom_size_bytes);

    }catch(std::exception& e){
        VNES_LOG::LOG(VNES_LOG::FATAL, "Failed to load ROM from file. Exception was: %s", e.what());
        VNES_ASSERT(0 && "Failed to load ROM from file");
    }

    file.close();


    //std::cout << "trainer.has_value() = " << trainer.has_value() << std::endl;
    //std::cout << "prg_rom.size() = " << prg_rom.size() << std::endl;
    //std::cout << "chr_rom.size() = " << chr_rom.size() << std::endl;
    //std::cout << "prg_rom_size_bytes = " << prg_rom_size_bytes << std::endl;
    //std::cout << "chr_rom_size_bytes = " << chr_rom_size_bytes << std::endl;
    //std::cout << "prg_ram_size_bytes = " << prg_ram_size_bytes << std::endl;
    //std::cout << "mapper = " << mapper_number << std::endl;
    //std::cout << "nametable_layout = " << nametable_layout << std::endl;
    //std::cout << "flags_6 = " << std::bitset<8>(header.data.flags_6) << std::endl;
    //std::cout << "flags_7 = " << std::bitset<8>(header.data.flags_7) << std::endl;

}

void Cartridge::parse_iNES_header(){
    // mapper
    uint8_t mapper_number_low_nybble = (header.data.flags_6 & 0xF0) >> 4;
    uint8_t mapper_number_high_nybble = (header.data.flags_7 & 0xF0);
    if(header.data.cpu_ppu_timing | header.data.hardware_type | header.data.misc_roms_present | header.data.default_expansion_device){
        // if last 4 bytes are not zero and ROM is marked for iNES format, it means there may
        // have been garbage data in the header (including in flags_7, which did not use
        // to be part of the spec. In this case, we should mask the upper nybble of the mapper number
        // see https://www.nesdev.org/wiki/INES#Flags_10
        mapper_number_high_nybble = 0;
    }
    mapper_number = mapper_number_high_nybble | mapper_number_low_nybble;


    // trainer
    bool trainer_present = header.data.flags_6 & 0x04;
    if(trainer_present){
        trainer.reset();
        trainer.emplace();
    }

    // PRG-ROM
    uint8_t prg_rom_size_x16KiB = header.data.prg_rom_size_lsb;
    prg_rom_size_bytes = prg_rom_size_x16KiB * 16384;

    // CHR-ROM
    uint8_t chr_rom_size_x8KiB = header.data.chr_rom_size_lsb;
    chr_rom_size_bytes = chr_rom_size_x8KiB * 8192;
    
    // nametable layout
    nametable_layout = (header.data.flags_6 & 0x01) ? HORIZONTAL : VERTICAL;
    // bit 3 of flags_6 also specifies whether an alternate nametable is being used,
    // but its being ignored here since mappers using that feature aren't supported

    // PRG-RAM
    uint8_t prg_ram_size_x8KiB = header.data.mapper_msb_submapper; // byte 8 is prg_ram size in iNES, mapper #'s in iNES2
    if(prg_ram_size_x8KiB == 0){
        prg_ram_size_x8KiB = 1; // if byte is zero, 8KiB are assumed present by default. see https://www.nesdev.org/wiki/INES#Flags_8
    }
    prg_ram_size_bytes = prg_ram_size_x8KiB * 8192;

}

void Cartridge::parse_iNES2_header(){
        // mapper
        uint16_t mapper_number_low_nybble = (header.data.flags_6 & 0xF0) >> 4;
        uint16_t mapper_number_middle_nybble = (header.data.flags_7 & 0xF0) >> 4;
        uint16_t mapper_number_high_nybble = (header.data.mapper_msb_submapper & 0x0F);
        mapper_number = (mapper_number_high_nybble << 8) | (mapper_number_middle_nybble << 4) | mapper_number_low_nybble;
        submapper_number = (header.data.mapper_msb_submapper & 0xF0) >> 4;

        // trainer
        bool trainer_present = header.data.flags_6 & 0x04;
        if(trainer_present){
            trainer.reset();
            trainer.emplace();
        }

        // PRG-ROM
        uint8_t prg_rom_lsb = header.data.prg_rom_size_lsb;
        uint8_t prg_rom_msb = header.data.prg_chr_rom_msb & 0x0F;

        // -> ((uint16_t)prg_rom_msb) << 8
        //  = ((uint16_t)0x0F) << 8 
        //  =  (0x000F) << 8 
        //  =   0x0F00
        uint16_t prg_rom_size_specifier = (((uint16_t)prg_rom_msb) << 8) | prg_rom_lsb; // may in interpreted as raw value or as exponent

        if(prg_rom_msb == 0x0F){
            // exponent notation, see https://www.nesdev.org/wiki/NES_2.0#PRG-ROM_Area
            uint8_t multipler = (prg_rom_lsb & 0x03)*2 + 1;
            uint8_t exponent = (prg_rom_lsb >> 2);

            prg_rom_size_bytes = (uint32_t)std::pow(2, exponent) * (uint32_t)multipler;
        }else{
            // simply size in 16KiB chunks
            uint16_t prg_rom_size_x16KiB = prg_rom_size_specifier;
            prg_rom_size_bytes = prg_rom_size_x16KiB * 16384; // 16KiB = 16384 bytes
        }


        // CHR-ROM
        uint8_t chr_rom_lsb = header.data.chr_rom_size_lsb;
        uint8_t chr_rom_msb = header.data.prg_chr_rom_msb & 0xF0;

        // -> ((uint16_t)chr_rom_msb) << 8
        //  = ((uint16_t)0xF0) << 4 
        //  =  (0x00F0) << 4 
        //  =   0x0F00
        uint16_t chr_rom_size_specifier = (((uint16_t)chr_rom_msb) << 4) | chr_rom_lsb; // may in interpreted as raw value or as exponent

        if(chr_rom_msb == 0xF0){
            // exponent notation, see https://www.nesdev.org/wiki/NES_2.0#CHR-ROM_Area
            uint8_t multipler = (chr_rom_lsb & 0x03)*2 + 1;
            uint8_t exponent = (chr_rom_lsb >> 2);

            chr_rom_size_bytes = (uint32_t)std::pow(2, exponent) * (uint32_t)multipler;
        }else{
            // simply size in 8KiB chunks
            uint16_t chr_rom_size_x8KiB = chr_rom_size_specifier;
            chr_rom_size_bytes = chr_rom_size_x8KiB * 8192; // 8KiB = 8192 bytes
        }

        // nametable layout
        nametable_layout = (header.data.flags_6 & 0x01) ? HORIZONTAL : VERTICAL;
        // bit 3 of flags_6 also specifies whether an alternate nametable is being used,
        // but its being ignored here since mappers using that feature aren't supported

        // PRG-(NV)RAM/EEPROM
        // see https://www.nesdev.org/wiki/NES_2.0#PRG-(NV)RAM/EEPROM
        uint8_t prg_ram_shift_count = header.data.prg_ram_eeprom_shift & 0x0F;
        if(prg_ram_shift_count){
            prg_ram_size_bytes = 64 << prg_ram_shift_count;
        }

        uint8_t eeprom_shift_count = header.data.prg_ram_eeprom_shift & 0xF0;
        if(eeprom_shift_count){
            eeprom_size_bytes = 64 << (eeprom_shift_count >> 4);
        }

        
        // CHR-(NV)RAM
        // see https://www.nesdev.org/wiki/NES_2.0#CHR-(NV)RAM
        uint8_t chr_ram_shift_count = header.data.chr_ram_shift & 0x0F;
        if(chr_ram_shift_count){
            chr_ram_size_bytes = 64 << chr_ram_shift_count;
        }

        uint8_t chr_nvram_shift_count = header.data.chr_ram_shift & 0xF0;
        if(chr_nvram_shift_count){
            chr_nvram_size_bytes = 64 << (chr_nvram_shift_count >> 4);
        }

        uint8_t region = header.data.cpu_ppu_timing & 0x03; // 0: NTSC = North America, Japan, South Korea, Taiwan
                                                            // 1: PAL = Western Europe, Australia
                                                            // 2: Multiple  Multiple
                                                            // 3: Dendy = Eastern Europe, Russia, Mainland China, India, Africa
        if(region != 0 && region != 2){ 
            VNES_LOG::LOG(VNES_LOG::ERROR, "Detected non-NTSC/multiple region cartridge region %d. Will treat ROM as NTSC, but this may cause issues!", region);
        }

        // Note: Header byte 13 is ignored since VS. systems/other extended hardware types are not supported

        uint8_t default_expansion_device = header.data.default_expansion_device & 0x3F;
        if(default_expansion_device != 0 && default_expansion_device != 1){
            VNES_LOG::LOG(VNES_LOG::ERROR, "Detected non-standard (normal controller) expansion device 0x%02x. Will run anyway, but the game probably won't work without this peripheral", default_expansion_device);
        }

}

void Cartridge::dump_rom(){
    for(unsigned int i = 0x0000; i < prg_rom.size(); i++){
        int data1 = prg_rom[i];
        int data2 = prg_rom[i];
        i += 0x8000;
        if(i && !(i % 2)){ printf(" "); }
        if(!(i % 32)){ 
            printf("\n0x");
            if(!(i & 0xF000)){ printf("0"); }
            if(!(i & 0xFF00)){ printf("0"); }
            if(!(i & 0xFFF0)){ printf("0"); }
            printf("%x  ", i); 
        }

        //int data = read(i);
        if(!(data1 & 0xF0)){ printf("0"); }
        printf("%x", data2);
        i -= 0x8000;
    }
    printf("\n");
}

uint8_t Cartridge::read(uint16_t addr){
    using namespace VNES_LOG;
    switch(addr){
        case 0x0000 ... 0x401F:
            LOG(ERROR, "Cartridge memory read at out-of-bounds address 0x%x (expected range is 0x4020 to 0xFFFF). Returning 0x0", addr);
            return 0;
            break;
        case 0x4020 ... 0xFFFF:
            return mapper->read(addr);
            break;
        default:
            LOG(ERROR, "Cartridge memory read at out-of-bounds address 0x%x (expected range is 0x4020 to 0xFFFF). Returning 0x0", addr);
            return 0;
            break;
    }
}

uint8_t Cartridge::read_pattern_table(uint16_t addr){
    addr &= 0x3FFF;
    if(addr > chr_rom.size() || !chr_rom.size()){
        VNES_LOG::LOG(VNES_LOG::ERROR, "Out of bounds CHR ROM read at address 0x%x in CHR ROM of size 0x%x", addr, chr_rom.size());
        addr %= chr_rom.size();
    }
    return chr_rom[addr];
}

void Cartridge::write_pattern_table(uint16_t addr, uint8_t data){
    addr &= 0x3FFF;
    if(addr > chr_rom.size() || !chr_rom.size()){
        VNES_LOG::LOG(VNES_LOG::ERROR, "Out of bounds CHR ROM write at address 0x%x with data 0x%x in CHR ROM of size 0x%x", addr, data, chr_rom.size());
        addr %= chr_rom.size();
    }
    chr_rom[addr] = data;
}

void Cartridge::write(uint16_t addr, uint8_t data){
    using namespace VNES_LOG;
    switch(addr){
        case 0x0000 ... 0x401F:
            LOG(ERROR, "Cartridge memory write at out-of-bounds address 0x%x (expected range is 0x4020 to 0x8000). Write has no effect", addr);
            break;
        case 0x4020 ... 0x7FFF:
            mapper->write(addr, data);
            break;
        case 0x8000 ... 0xFFFF:
            LOG(WARN, "Cartridge memory write to read-only address 0x%x (expected range is 0x4020 to 0x8000). Write will be allowed but should be noted", addr);
            mapper->write(addr, data);
            break;
        default:
            LOG(ERROR, "Cartridge memory write at out-of-bounds address 0x%x (expected range is 0x4020 to 0x8000). Write has no effect", addr);
            break;
    }
}

