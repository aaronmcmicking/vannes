#pragma once

#include "include/RAM.hpp"
#include "../common/nes_assert.hpp"
#include "../common/log.hpp"
#include <stdlib.h>
#include <fstream>
#include <stdio.h> // FILE* cause i dont like fstream 

RAM::RAM(Cartridge& cart): cart {cart} {
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing RAM...");
    // init RAM to 0, although this shouldn't be relied upon as actual reset
    // values were undefined (and unpredictable) in real hardware
    for(int i = 0; i < INTERNAL_MEM_ITEMS; i++){
        mem[i] = 0;
    }
}

void RAM::write(uint16_t addr, uint8_t data){
    switch(addr){
        case 0x0000 ... 0x1FFF: // 2kb program RAM, 4 mirrored sections (each 0x0800 addrs)
            addr = (addr % 0x0800);
            mem[addr] = data;
            break;
        case 0x2000 ... 0x3FFF: // PPU registers
            addr = (addr % 0x8) + 0x2000; // mirrors every eight bytes
            mem[addr] = data;
        case 0x4000 ... 0x401F: // APU and I/O registers
            break;
        case 0x4020 ... 0xFFFF: // cartridge ROM
            VNES_LOG::LOG(VNES_LOG::WARN, "RAM.write(): Usually cannot write to read-only ROM address 0x%x, maybe the mapper is allowing this?");
            cart.write(addr, data);
            break;
        default: // unreachable
            VNES_LOG::LOG(VNES_LOG::FATAL, "RAM.write(): Bad address 0x%x could not be mapped to mapper or internal RAM! How is this possible??");
            exit(1);
            break;
    }
}

uint8_t RAM::read(uint16_t addr){
    using namespace VNES_LOG;
    switch(addr){
        case 0x0000 ... 0x401F:
            LOG(DEBUG, "Read value 0x%x from address 0x%x (internal RAM)", mem[addr], addr);
            return mem[addr];
            break;
        case 0x4020 ... 0xFFFF:
            LOG(DEBUG, "Read value 0x%x from address 0x%x (cartridge)", cart.read(addr), addr);
            return cart.read(addr);
            break;
        default:
            LOG(FATAL, "RAM.read(): Bad address 0x%x could not be mapped to mapper or internal RAM! How is this possible??");
            exit(1);
            break;
    }
}

void RAM::dump(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Dumping RAM as accessed by RAM::read() (not resilient to cartridge/mapper/RAM bugs)");
    FILE* file = fopen("ram.dump", "w");
    //std::ofstream file {};
    //file.open("ram.dump", std::ios::out);

    VNES_LOG::Severity old_log_level = VNES_LOG::log_level;
    VNES_LOG::log_level = VNES_LOG::ERROR; // disable DEBUG/INFO/WARN 
    for(int i = 0x0000; i <= 0xFFFF; i++){
        if(i && !(i % 2)){ fprintf(file, " "); }
        if(!(i % 32)){ 
            fprintf(file, "\n0x");
            if(!(i & 0xF000)){ fprintf(file, "0"); }
            if(!(i & 0xFF00)){ fprintf(file, "0"); }
            if(!(i & 0xFFF0)){ fprintf(file, "0"); }
            fprintf(file, "%x  ", i); 
        }

        int data = read(i);
        if(!(data & 0xF0)){ fprintf(file, "0"); }
        fprintf(file, "%x", read(i));
    }
    fprintf(file, "\n");
    fclose(file);
    VNES_LOG::log_level = old_log_level; // re-enable logging
}
