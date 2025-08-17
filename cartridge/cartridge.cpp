#pragma once

#include <cmath>
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

/*
Cartridge::Cartridge(): mapper {nullptr}{ // dummy cart with 16k PRG and 8k CHR available
    using namespace VNES_LOG;
    header.nes_title[0]             = 0x4E;       
    header.nes_title[1]             = 0x45;       
    header.nes_title[2]             = 0x53;       
    header.nes_title[3]             = 0x1A;       
    header.prg_rom_size_x16kb       = 2; 
    header.chr_rom_size_x8kb        = 1;  
    header.flags_6                  = 0;            
    header.flags_7                  = 0;            
    header.flags_8                  = 0;            
    header.flags_9                  = 0;            
    header.padding[0]               = 0;         
    header.padding[1]               = 0;         
    header.padding[2]               = 0;         
    header.padding[3]               = 0;         
    header.padding[4]               = 0;         
    header.padding[5]               = 0;         

    trainer.reset();

    // get prg rom
    LOG(INFO, "Loading empty PRG ROM.", header.prg_rom_size_x16kb);
    prg_rom.clear();
    for(int i = 0; i < header.prg_rom_size_x16kb*16384; i++){
        prg_rom.insert(prg_rom.end(), 5); 
    }
    LOG(DEBUG, "header.prg_rom_size_x16kb is %d: loaded %d bytes to prg_rom", header.prg_rom_size_x16kb, prg_rom.size());

    // get chr rom
    LOG(INFO, "Loading empty CHR ROM");
    chr_rom.clear();
    for(int i = 0; i < header.chr_rom_size_x8kb*8192; i++){
        chr_rom.insert(chr_rom.end(), i % 0x100); 
    }
    LOG(DEBUG, "header.chr_rom_size_x8kb is %d: loaded %d bytes to chr_rom", header.chr_rom_size_x8kb, chr_rom.size());

    set_mapper();
}
*/


void Cartridge::set_mapper(){
    switch(mapper_number){
        case 0:
            mapper = make_unique<Mapper000>(prg_rom, chr_rom);
            break;
        default:
            mapper = make_unique<Mapper000>(prg_rom, chr_rom);
            VNES_LOG::LOG(VNES_LOG::WARN, "Mapper number %d not recognized, setting default %s", mapper_number, mapper->name.c_str());
            break;
    }
    VNES_LOG::LOG(VNES_LOG::INFO, "Set cartridge mapper to %s", mapper->name.c_str());
}


void Cartridge::load_rom(std::string filename){
    using namespace VNES_LOG;

    LOG(INFO, "Loading ROM.");

    std::ifstream file {};
    file.open(filename, std::ios::binary | std::ios::in);
    
    if(!file.is_open()){
        LOG(FATAL, "Failed to open ROM file to read. Exiting");
        VNES_ASSERT(0 && "Failed to open ROM file for reading");
    }else{
        LOG(INFO, "Opened ROM file: %s", filename.c_str());
    }

    Header header {file};

    if(  !(header.data.nes_title[0] == 0x4E  // 'N' 
        && header.data.nes_title[1] == 0x45  // 'E'
        && header.data.nes_title[2] == 0x53  // 'S'
        && header.data.nes_title[3] == 0x1A) // MS DOS end-of-line
    ){
        LOG(WARN, "Didn't find 'NES' at start of ROM file as expected by iNES standard. Instead got %x %x %x %x. Is the ROM malformed? Continuing anyway, but this may be a bad ROM file!", header.data.nes_title[0], header.data.nes_title[1], header.data.nes_title[2], header.data.nes_title[3]);
    }


    if(header.header_format == Header::iNES){
        // same as old parsing
        LOG(FATAL, "iNES header parsing not supported (use iNES2)");
    }else if(header.header_format == Header::iNES2){
        // see https://www.nesdev.org/wiki/NES_2.0

        // mapper
        uint16_t mapper_number_low_nybble = (header.data.flags_6 & 0xF0) >> 4;
        uint16_t mapper_number_middle_nybble = (header.data.flags_7 & 0xF0) >> 4;
        uint16_t mapper_number_high_nybble = (header.data.mapper_msb_submapper & 0x0F);
        mapper_number = (mapper_number_high_nybble << 8) | (mapper_number_middle_nybble << 4) | mapper_number_low_nybble;
        submapper_number = (header.data.mapper_msb_submapper & 0xF0) >> 4;

        // trainer
        bool trainer_present = header.data.flags_6 & 0x04;
        if(trainer_present){
            // trainer is a 512 byte region immediately after the header
            // and before the PRG/CHR data that should be loaded into address
            // 0x7000 in CPU memory. This is due to some workarounds for different
            // cartrdige hardware.

            trainer.reset();
            trainer.emplace();

            std::array<uint8_t, 512> trainer_array = trainer.value();
            for(int i = 0; i < 512; i++){
                trainer.value()[i] = file.get();
            }
        }else{
            trainer.reset();
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

    } // iNes2 parsing

    LOG(DEBUG, "Finished parsing header");

    // Read trainer, PRG, CHR data in from file
    LOG(FATAL, "Unimplemented data fetch");
    file.seekg(16); // make sure we are at the end of the header

    file.close();

    VNES_ASSERT(0);
}

/*
void Cartridge::load_rom_old(std::string filename){
    using namespace VNES_LOG;
    LOG(INFO, "Loading ROM.");
    std::ifstream file {};
    file.open(filename, std::ios::binary | std::ios::in);
    LOG(INFO, "Loading ROM: %s", filename.c_str());
    //std::cout << "Loading ROM: " << filename << std::endl;
    
    if(!file.is_open()){
        LOG(FATAL, "Failed to open ROM file to read. Exiting");
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
        LOG(WARN, "Didn't find 'NES' at start of ROM file as expected by iNES standard. Instead got %x %x %x %x. Is the ROM malformed? Continuing with execution, but there may have actually been a critical error!", header.nes_title[0], header.nes_title[1], header.nes_title[2], header.nes_title[3]);
    }

    // check header format
    if(   // if NOT in NES2 format and final 4 bytes of header are NOT zeroes
          !((header.flags_7 | 0b00001100) == 0b00001000) // "If [bits[3:2]] equal to 2, flags 8-15 are in NES 2.0 format"
       && (header.padding[2] | header.padding[3] | header.padding[4] | header.padding[5])){
        LOG(INFO, "Found data in final 4 bytes of cartridge header when header did not select NES2 standard. Sometimes this indicates the ROM complies to an old version of the iNES standard, but may also mean the ROM is malformed or 'authour-signatured'. Clearing mapper number to bottom 4 bits and continuing");
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
    LOG(INFO, "Loading PRG ROM.", header.prg_rom_size_x16kb);
    prg_rom.clear();
    for(int i = 0; i < header.prg_rom_size_x16kb*16384; i++){
        prg_rom.insert(prg_rom.end(), file.get()); 
    }
    LOG(DEBUG, "header.prg_rom_size_x16kb is %d: loaded %d bytes to prg_rom", header.prg_rom_size_x16kb, prg_rom.size());

    // get chr rom
    LOG(INFO, "Loading CHR ROM");
    chr_rom.clear();
    for(int i = 0; i < header.chr_rom_size_x8kb*8192; i++){
        chr_rom.insert(chr_rom.end(), file.get()); 
    }
    LOG(DEBUG, "header.chr_rom_size_x8kb is %d: loaded %d bytes to chr_rom", header.chr_rom_size_x8kb, chr_rom.size());

    file.close();
    LOG(INFO, "Finished loading ROM.");
}
*/


//void Cartridge::dump_rom(){
//    VNES_LOG::LOG(VNES_LOG::INFO, "Dumping PRG ROM");
//    int count = 0;
//    for(uint8_t byte_: prg_rom){
//        printf("%2x", byte_);
//        if(count && !(count % 2)) printf(" ");
//        if(count && !(count % 16)) printf("\n");
//        count++;
//    }
//    printf("\n\n");
//
//    VNES_LOG::LOG(VNES_LOG::INFO, "Dumping CHR ROM");
//    count = 0;
//    for(uint8_t byte_: chr_rom){
//        printf("%2x", byte_);
//        if(count && !(count % 2)) printf(" ");
//        if(count && !(count % 16)) printf("\n");
//        count++;
//    }
//    printf("\n");
//}

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

uint8_t Cartridge::read_pallete(uint16_t addr){
    addr &= 0x3FFF;
    if(!chr_rom.size()){
        VNES_LOG::LOG(VNES_LOG::ERROR, "Tried to read address 0x%x from empty CHR ROM", addr);
    }
    if(addr > chr_rom.size()){
        VNES_LOG::LOG(VNES_LOG::ERROR, "Out of bounds CHR ROM read at address 0x%x in CHR ROM of size 0x%x", addr, chr_rom.size());
        addr %= chr_rom.size();
    }
    return chr_rom.at(addr);
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

