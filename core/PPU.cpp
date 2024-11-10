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
    if(addr == PPU_CTRL){
        data = internal_read(PPU_CTRL) & 0x7F; // set nmi_output
    }
    ram.write(addr, data);
}

uint8_t PPU::internal_read(uint16_t addr){
    uint8_t data = ram.read(addr);
    if(addr == PPU_STATUS){
        internal_write(addr, data & 0x7F); // clear nmi_occured, return old value, 'vblank flag'
    }
    return data;
}

bool PPU::check_nmi(){
    return (internal_read(PPU_CTRL) & 0x80) && (internal_read(PPU_STATUS) & 0x80);
}

void PPU::power_up(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Powering up PPU");
    reset();
    write(PPU_OAM_ADDR, 0x00);
    write(PPU_ADDR, 0x00);
    write(PPU_DATA, 0x00);
    VNES_LOG::LOG(VNES_LOG::INFO, "Done powering up PPU");
}

void PPU::reset(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Resetting PPU");
    cycles_since_reset = 0;
    frame_cycles = 0;
    scanline_cycles = 0;
    scanline = 0;
    dot = 0;
    //for(int x = 0; x < 256; x++){
    //    for(int y = 0; y < 224; y++){
    //        buffer[x][y] = 0;
    //    }
    //}
    for(int i = 0; i < 256*224; i++){
        buffer[i] = 0;
    }
    //vblank = false;
    frame_done = false;
    odd_frame = false;
    write(PPU_CTRL, 0x00);    
    write(PPU_MASK, 0x00);    
    write(PPU_STATUS, 0x00);    
    write(PPU_SCROLL, 0x00);
    write(PPU_DATA, 0x00);
    VNES_LOG::LOG(VNES_LOG::INFO, "PPU reset done");
}

void PPU::do_cycles(int cycles_to_do){
    //VNES_LOG::LOG(VNES_LOG::DEBUG, "PPU cycle requested");
    for(int i = 0; i < cycles_to_do; i++){
        cycles_since_reset++;
        frame_cycles++;
        if(scanline_cycles == 341){
            scanline_cycles = 0;
            scanline++;
            if(scanline > 262){
                scanline = -1;
                // TODO: signal frame complete
                //vblank = true;
            }
        }else{
            scanline_cycles++;
        }

        if(scanline == 241 && dot == 1){ // start of vBlank
            internal_write(PPU_STATUS, internal_read(PPU_STATUS) | 0x80);
        }else if(scanline == 261 && dot == 1){ // end of vBlank
            // at dot 1 (second dot) of pre-render line (20 lines after start of vBlank)
            // clear all of PPU_STATUS except PPU open bus
            internal_write(PPU_STATUS, internal_read(PPU_STATUS) & 0x1F);
        }

    }
}


