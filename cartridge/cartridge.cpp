#pragma once

#include <fstream>
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

void Cartridge::set_mapper(){
    switch(mapper_number){
        case 0:
            mapper = make_unique<Mapper000>(prg_rom, chr_rom);
            break;
        default:
            VNES_LOG::LOG(VNES_LOG::WARN, "Mapper number %d not recognized, setting default Mapper000", mapper_number);
            mapper = make_unique<Mapper000>(prg_rom, chr_rom);
            break;
    }
    VNES_LOG::LOG(VNES_LOG::INFO, "Set cartridge mapper to %s", mapper->name.c_str());
}

void Cartridge::load_rom(std::string filename){
    VNES_LOG::LOG(VNES_LOG::INFO, "Loading ROM.");
    std::ifstream file {};
    file.open(filename, std::ios::binary | std::ios::in);
    std::cout << '\'' << filename << '\'' << std::endl;
    
    if(!file.is_open()){
        VNES_LOG::LOG(VNES_LOG::FATAL, "Failed to open ROM file to read. Exiting");
        VNES_ASSERT(0 && "Failed to open ROM file for reading");
    }

    header.nes_title[0]             = file.get();       
    header.nes_title[1]             = file.get();       
    header.nes_title[2]             = file.get();       
    header.nes_title[3]             = file.get();       
    header.prg_rom_size_x16kb       = file.get(); 
    header.chr_rom_size_x8kb        = file.get();  
    header.flags_6                  = file.get();            
    header.flags_7                  = file.get();            
    header.flags_8                  = file.get();            
    header.flags_9                  = file.get();            
    header.padding[0]               = file.get();         
    header.padding[1]               = file.get();         
    header.padding[2]               = file.get();         
    header.padding[3]               = file.get();         
    header.padding[4]               = file.get();         
    header.padding[5]               = file.get();         

    // check header tag
    if(  !(header.nes_title[0] == 0x4E      
        && header.nes_title[1] == 0x45 
        && header.nes_title[2] == 0x53  
        && header.nes_title[3] == 0x1A) // 0x1A is MS DOS end-of-line
    ){
        VNES_LOG::LOG(VNES_LOG::WARN, "Didn't find 'NES' at start of ROM file as expected by iNES standard. Instead got %x %x %x %x. Is the ROM malformed? Continuing with execution, but there may have actually been a critical error!", header.nes_title[0], header.nes_title[1], header.nes_title[2], header.nes_title[3]);
    }

    // check header format
    if(   // if NOT in NES2 format and final 4 bytes of header are NOT zeroes
          !((header.flags_7 | 0b00001100) == 0b00001000) // "If [bits[3:2]] equal to 2, flags 8-15 are in NES 2.0 format"
       && !(header.padding[2] | header.padding[3] | header.padding[4] | header.padding[5])){
        VNES_LOG::LOG(VNES_LOG::INFO, "Found data in final 4 bytes of cartridge header when not in NES2 mode. Sometimes this indicates a malformed or 'authour-signatured', but may also mean the ROM complies to an old version of the iNES standard. Clearing mapper number to bottom 4 bits and continuing");
        header.flags_7 &= 0b00001111;
    }

    // set mapper number
    mapper_number = (header.flags_7 & 0b11110000) | (header.flags_6 >> 4);

    // get trainer
    bool trainer_present = header.flags_6 & 0b00000100;
    if(trainer_present){
        trainer.emplace();
        for(int i = 0; i < 512; i++){
            trainer.value()[i] = file.get();
        }
    }else{
        trainer.reset();
    }

    // get prg rom
    VNES_LOG::LOG(VNES_LOG::INFO, "Loading PRG ROM.", header.prg_rom_size_x16kb);
    prg_rom.clear();
    for(int i = 0; i < header.prg_rom_size_x16kb*16384; i++){
        prg_rom.insert(prg_rom.end(), file.get()); 
    }
    VNES_LOG::LOG(VNES_LOG::DEBUG, "header.prg_rom_size_x16kb is %d: loaded %d bytes to prg_rom", header.prg_rom_size_x16kb, prg_rom.size());

    // get chr rom
    VNES_LOG::LOG(VNES_LOG::INFO, "Loading CHR ROM");
    chr_rom.clear();
    for(int i = 0; i < header.chr_rom_size_x8kb*8192; i++){
        chr_rom.insert(chr_rom.end(), file.get()); 
    }
    VNES_LOG::LOG(VNES_LOG::DEBUG, "header.chr_rom_size_x8kb is %d: loaded %d bytes to chr_rom", header.chr_rom_size_x8kb, chr_rom.size());

    file.close();
    VNES_LOG::LOG(VNES_LOG::INFO, "Finished loading ROM.");
}

void Cartridge::dump_rom(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Dumping PRG ROM");
    int count = 0;
    for(uint8_t byte_: prg_rom){
        printf("%2x", byte_);
        if(count && !(count % 2)) printf(" ");
        if(count && !(count % 16)) printf("\n");
        count++;
    }
    printf("\n\n");

    VNES_LOG::LOG(VNES_LOG::INFO, "Dumping CHR ROM");
    count = 0;
    for(uint8_t byte_: chr_rom){
        printf("%2x", byte_);
        if(count && !(count % 2)) printf(" ");
        if(count && !(count % 16)) printf("\n");
        count++;
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
            LOG(ERROR, "Cartridge memory write to read-only address 0x%x (expected range is 0x4020 to 0x8000). Write has no effect", addr);
            break;
        default:
            LOG(ERROR, "Cartridge memory write at out-of-bounds address 0x%x (expected range is 0x4020 to 0x8000). Write has no effect", addr);
            break;
    }
}

