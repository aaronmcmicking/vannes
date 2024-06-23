#pragma once

#include "include/PPU.hpp"
#include "../common/log.hpp"
#include <algorithm>

PPU::PPU(RAM& _ram): ram {_ram}{ 
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing PPU");
    power_up();
    VNES_LOG::LOG(VNES_LOG::INFO, "Done constructing PPU");
}

void PPU::write(uint16_t addr, uint8_t data){
    std::vector<PPU_regs> write_locked_after_reset {PPU_CTRL, PPU_MASK, PPU_SCROLL, PPU_ADDR};
    // some registers are write-locked for about 30,000 CPU cycles after reset
    if(cycles_since_reset && (cycles_since_reset / 3) < 29658 
            && std::find(write_locked_after_reset.begin(), write_locked_after_reset.end(), addr) != write_locked_after_reset.end())
    {
        VNES_LOG::LOG(VNES_LOG::DEBUG, "Write to PPU register address 0x%x ignored at %lld CPU cycles after reset", addr, cycles_since_reset / 3);
        return;
    }

    // TODO: check for read-only registers
    internal_write(addr, data);
}

uint8_t PPU::read(uint16_t addr){
    // TODO: check for write-only registers
    return internal_read(addr);
}

void PPU::internal_write(uint16_t addr, uint8_t data){
    ram.write(addr, data);
}

uint8_t PPU::internal_read(uint16_t addr){
    return ram.read(addr);
}

void PPU::power_up(){
    frame_cycles = 0;
    scanline_cycles = 0;
    scanlines = 0;
    write(PPU_CTRL, 0x00);    
    write(PPU_MASK, 0x00);    
    write(PPU_STATUS, 0x00);    
    write(PPU_OAM_ADDR, 0x00);
    write(PPU_SCROLL, 0x00);
    write(PPU_ADDR, 0x00);
    write(PPU_DATA, 0x00);
}

void PPU::reset(){
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Reseting PPU");
    cycles_since_reset = 0;
    frame_cycles = 0;
    scanline_cycles = 0;
    scanlines = 0;
    write(PPU_CTRL, 0x00);    
    write(PPU_MASK, 0x00);    
    write(PPU_STATUS, 0x00);    
    write(PPU_SCROLL, 0x00);
    write(PPU_DATA, 0x00);
    VNES_LOG::LOG(VNES_LOG::DEBUG, "PPU reset done");
}

void PPU::step(){
    //VNES_LOG::LOG(VNES_LOG::DEBUG, "PPU cycle requested");
    cycles_since_reset++;
    frame_cycles++;
    if(scanline_cycles == 341){
        scanline_cycles = 0;
        scanlines++;
        if(scanlines > 260){
            scanlines = -1;
            // TODO: signal frame complete
        }
    }else{
        scanline_cycles++;
    }
}


