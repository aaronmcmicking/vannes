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
    frame_cycle = 0;
    scanline_cycle = 0;
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
        cycle();
    }
}

void PPU::cycle(){
    // see https://www.nesdev.org/wiki/PPU_rendering

    int scanline_cycle_modulo  = 0;
    switch(scanline){

        //case 0 ... 239:
        case -1 ... 239:
            // pre-render line and visible lines

            /*
             * The dummy line, -1 (aka 261) is inluded in this block since it
             * behaves the same as scanlines 0-239 (loads the tile data into 
             * the shift registers for scanline 0), but doesn't render
             */
            
            switch(scanline_cycle){
                case 0:
                    // idle cycle
                    break;

                case 1 ... 256:
                    // fetch tile data
                    
                    /*
                     * At the start of every scanline, the first two tiles are 
                     * already loaded/ready for rendering since the previous 
                     * scanline loads them. So the tile 3 is the first tile 
                     * loaded this scanline.
                     *
                     * Loading a tile requires 4 memory access at 2 cycles/dots per access
                     * (8 dots per tile). The 4 access are:
                     *     - Nametable byte
                     *     - Attribute byte
                     *     - Pattern table tile low
                     *     - Pattern table tile high ("+8 bytes from patter table tile low")
                     *
                     * Every 8 cycles, this data is loaded to the appropriate shift registers.
                     * This also means each 8 sequential pixels must have the same palette attribute
                     *
                     * Sprite 0 hit acts as if image starts on cycle 2 (same 
                     * cycle shift register shifts for first time), and so 
                     * sprite 0 flag raised on cycle 2 at the earliest. Due 
                     * to render pipelining, first pixel is output on cycle 4.
                     *
                     * Then, shifters are reloaded on cycles 9, 17, 25, ..., 257.
                     * 
                     * Also, sprite evaluation for next scanline occurs parallel/independant to these cycles
                     * 
                     */

                    if(scanline_cycle == 2){
                        // sprite 0 hit
                    }

                    if(scanline_cycle >= 9 && (scanline_cycle % 8) == 1){
                        // reload shift registers every 8 cycles
                    }

                    scanline_cycle_modulo = ((scanline_cycle-1) % 8) + 1; // scanline_cycle mod 8, rolling over to 1 instead of 0
                    switch(scanline_cycle_modulo){ 
                        case 1 ... 2:
                            // nametable byte
                            break;

                        case 3 ... 4:
                            // attribute byte
                            break;

                        case 5 ... 6:
                            // pattern table tile low byte
                            break;

                        case 7 ... 8:
                            // pattern table tile high byte
                            break;

                        default:
                            VNES_LOG::LOG(VNES_LOG::ERROR, "PPU reached bad memory fetch: tried to do memory fetch for cycle %d: cycle %d, scanline %d", scanline_cycle, dot, scanline);
                            break;

                    }

                    break;

                case 257 ... 320:
                    // fetch sprite tile data for next scanline

                    /*
                     * Need to do the same 4 accesses as scanlines 0-256 for each of the 8 sprites:
                     *     - Garbage nametable byte
                     *     - Garbage attribute byte
                     *     - Pattern table tile low
                     *     - Pattern table tile high ("+8 bytes from patter table tile low")
                     *
                     */
                    break;

                case 321 ... 336:
                    // fetch first two tiles for next scanline and load into shift registers
                    break;

                case 337 ... 340:
                    // fetch 2 unused bytes, both the same nametable as fetched 
                    // at the start of the next scanline (MMC5 mapper uses this 
                    // sequence to count scanlines)
                    break;

                default:
                    VNES_LOG::LOG(VNES_LOG::ERROR, "PPU reached bad scanline_cycle: dot %d, scanline %d", dot, scanline);
                    break;
            }

            if(odd_frame && scanline == -1 && scanline_cycle == 339){
                scanline_cycle++; // scanline -1 skips scanline_cycle 340 on odd frames and jumps to scanline 0, scanline_cycle 0
            }
            
            break;

        case 240:
            // post-render line, idle scanline
            break;

        case 241 ... 260:
            // vertical blanking lines

            if(scanline == 241 && scanline_cycle == 1){
                // set vblank flag and attempt to raise NMI
            }
            break;

        //case 261:
        //    // pre-render line (aka scanline -1)
        //    break;

        default:
            VNES_LOG::LOG(VNES_LOG::ERROR, "PPU reached bad scanline: dot %d, scanline %d", dot, scanline);
            break;
    }

    scanline_cycle++;
    if(scanline_cycle > 340){
        scanline++;
        scanline_cycle = 0;
    }
    
    if(scanline > 260){
        scanline = -1;
    }

}

/*
void PPU::cycle(){
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
*/


