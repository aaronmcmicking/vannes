#pragma once

#include "include/PPU.hpp"
#include "../common/log.hpp"
#include <algorithm>

//PPU::PPU(RAM& _ram, Cartridge& _cart): ram {_ram}, cart {_cart} { 
PPU::PPU(Cartridge& _cart, DMABus& _dmabus): cart {_cart}, dmabus {_dmabus} { 
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing PPU");
    power_up();
    VNES_LOG::LOG(VNES_LOG::INFO, "Done constructing PPU");
}

uint8_t PPU::register_read(uint16_t addr){
    uint16_t mod_addr = (addr % 0x8) + 0x2000; // the RAM address range for PPU registers mirrors every 8 bytes

    uint8_t open_bus = 0; // reads from write-only regs should return latched bus value, see 'MMIO Registers' on https://www.nesdev.org/wiki/PPU_registers#MMIO_registers
    uint8_t data = 0;
    switch(mod_addr){
        case PPU_CTRL:	// W 	PPU Control 1
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to read from write-only register at address %u -> %u", addr, mod_addr);
            data = open_bus;
			break;
        case PPU_MASK:	// W 	PPU Control 2
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to read from write-only register at address %u -> %u", addr, mod_addr);
            data = open_bus;
			break;
        case PPU_STATUS:	// R 	PPU Status
            data = ppu_status;

            reg_w = 0; // reading PPU_STATUS resets the w register
            ppu_status &= 0x7F; // reading PPU_STATUS returns and then resets the value of the vblank flag
			break;
        case PPU_OAM_ADDR:	// W 	OAM Address
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to read from write-only register at address %u -> %u", addr, mod_addr);
            data = open_bus;
			break;
        case PPU_OAM_DATA:	// R/W 	OAM Data
            data = OAM_PRIMARY[ppu_oam_addr];
			break;
        case PPU_SCROLL:	// W 2x 	Background Scroll Position \newline (write X then Y)
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to read from write-only register at address %u -> %u", addr, mod_addr);
            data = open_bus;
			break;
        case PPU_ADDR:	// W 2x 	PPU Address \newline (write upper then lower)
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to read from write-only register at address %u -> %u", addr, mod_addr);
            data = open_bus;
			break;
        case PPU_DATA:	// R/W 	PPU Data
            data = ppu_data_read_buffer;
            ppu_data_read_buffer = vram[ppu_addr];

            if(ppu_ctrl | 0x04){ // after access, addr increments by 1 or 32, specified by bit 2 of PPU_CTRL
                ppu_addr += 32;
            }else{
                ppu_addr++;
            }
			break;
        case PPU_OAM_DMA:	// W 	Sprite Page DMA Transfer 
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to read from write-only register at address %u -> %u", addr, mod_addr);
            data = open_bus;
			break;
        default:
            VNES_LOG::LOG(VNES_LOG::ERROR, "Tried to read to bad PPU register address %u -> %u", addr, mod_addr);
            break;
    }

    return data;
}

void PPU::register_write(uint16_t addr, uint8_t data){
    uint16_t mod_addr = (addr % 0x8) + 0x2000; // the RAM address range for PPU registers mirrors every 8 bytes

    // Some registers are write-locked for about 29658 CPU cycles after reset
    // 29658*3 = 88974 ppu cycles
    //
    // Right now, this behaviour is hard-coded, but in original hardware it was
    // a side effect of the PPU's startup state (a flag being cleared a during a PPU operation
    // eventually causes the register to be writtable). Emulating that hardward side
    // effect would be more accurate.
    const std::vector<PPU_regs> write_locked_after_reset {PPU_CTRL, PPU_MASK, PPU_SCROLL, PPU_ADDR};
    if(cycles_since_reset < 88974 
            && std::find(write_locked_after_reset.begin(), write_locked_after_reset.end(), mod_addr) != write_locked_after_reset.end())
    {
        VNES_LOG::LOG(VNES_LOG::DEBUG, "Write to PPU register address 0x%x ignored at %lld CPU cycles after reset", addr, cycles_since_reset / 3);
        return;
    }

    // TODO: check for read-only registers
    switch(mod_addr){
        case PPU_CTRL:	// W 	PPU Control 1
            ppu_ctrl = data;
			break;
        case PPU_MASK:	// W 	PPU Control 2
            ppu_mask = data;
			break;
        case PPU_STATUS:	// R 	PPU Status
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to write to read-only register at address %u -> %u", addr, mod_addr);
			break;
        case PPU_OAM_ADDR:	// W 	OAM Address
            ppu_oam_addr = data;
			break;
        case PPU_OAM_DATA:	// R/W 	OAM Data
            OAM_PRIMARY[ppu_oam_addr] = data;
            ppu_oam_addr++; // writes automatically increment PPU_OAM_ADDR
			break;
        case PPU_SCROLL:	// W 2x 	Background Scroll Position \newline (write X then Y)
            // PPU_SCROLL is 16 bits wide writing to PPU_SCROLL 
            
            // t register? x register?
            if(!(reg_w & 0x1)){
                // w=0, first write, X scroll bits
                ppu_scroll &= 0xFF00; // preserve bits 15-8 and clear 7-0
                ppu_scroll |= data; // update 7-0
                reg_w = 1;
            }else{
                // w=1, second write, Y scroll bits
                ppu_scroll &= 0x00FF; // preserve bits 7-0 and clear 15-8
                ppu_scroll |= (data << 8); // update bits 15-8
                reg_w = 0;
            }
			break;
        case PPU_ADDR:	// W 2x 	PPU Address \newline (write upper then lower)
            // t register? v register?
            if(!(reg_w & 0x1)){
                // w=0, first write, high byte
                ppu_addr &= 0x00FF; // preserve bits 7-0 and clear 15-8
                ppu_scroll |= (data << 8); // update 15-8
                reg_w = 1;
            }else{
                // w=1, second write, low byte
                ppu_scroll &= 0xFF00; // preserve bits 15-8 and clear 7-0
                ppu_scroll |= data; // update bits 7-0
                reg_w = 0;
            }
			break;
        case PPU_DATA:	// R/W 	PPU Data
            // After access, the video memory address will increment by an amount determined by bit 2 of $2000. 
            // v register?
            vram[ppu_addr] = data;
            if(ppu_ctrl | 0x04){ // after access, addr increments by 1 or 32, specified by bit 2 of PPU_CTRL
                ppu_addr += 32;
            }else{
                ppu_addr++;
            }
			break;
        case PPU_OAM_DMA:	// W 	Sprite Page DMA Transfer 
			/* handler */

            // DMA is 256 pairs of READ FROM RAM (starting from address 0x[data]00) and writing to OAMDATA (will use current OAMADDR, programmer's responsibility to set proper starting address)
            for(int i = 0x0; i <= 0xFF; i++){
                // DMABus reads, OMADATA writes
                uint8_t current_addr = (data << 8) | i;
                register_write(PPU_OAM_DATA, dmabus.read(current_addr));
            }

            VNES_LOG::LOG(VNES_LOG::WARN, "PPU OAM DMA should add 514 cycles to the CPU");
            
			break;
        default:
            VNES_LOG::LOG(VNES_LOG::ERROR, "Tried to read to bad PPU register address %u", mod_addr);
            break;
    }
}

/*
 * TODO: PPU is never written to directly:
 *  CPU writes to PPU memory using PPU_ADDR and PPU_DATA
 *  CPU also writes to OAM using PPU_OAM_ADDR and PPU_OAM_DATA
 *  Cartridge mapper maps some of VRAM to CHR ROM
 *
 *  see:    https://www.nesdev.org/wiki/PPU_memory_map
 *          https://forums.nesdev.org/viewtopic.php?t=10297
 *          https://www.nesdev.org/wiki/CHR_ROM_vs._CHR_RAM
 *          https://www.nesdev.org/wiki/PPU_pattern_tables
 */
/*
void PPU::write(uint16_t addr, uint8_t data){
    const std::vector<PPU_regs> write_locked_after_reset {PPU_CTRL, PPU_MASK, PPU_SCROLL, PPU_ADDR};
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

    // reading OAM_DATA automatically increments OAM_ADDR
    if(addr == PPU_OAM_DATA){
        uint8_t oam_addr = ram.read(PPU_OAM_ADDR);
        oam_addr++;
        ram.write(PPU_OAM_ADDR, oam_addr);
    }
}

uint8_t PPU::internal_read(uint16_t addr){
    uint8_t data = ram.read(addr);
    if(addr == PPU_STATUS){
        internal_write(addr, data & 0x7F); // clear nmi_occured, return old value, 'vblank flag'
    }
    return data;
}
*/

bool PPU::check_nmi(){
    //return (internal_read(PPU_CTRL) & 0x80) && (internal_read(PPU_STATUS) & 0x80);
    return (ppu_ctrl & 0x80) && (ppu_status & 0x80);
}

void PPU::power_up(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Powering up PPU");
    reset();
    ppu_oam_addr = 0x00;
    ppu_addr = 0x00;
    ppu_data = 0x00;

    std::fill(std::begin(vram), std::end(vram), 0);

    VNES_LOG::LOG(VNES_LOG::INFO, "Done powering up PPU");
}

void PPU::reset(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Resetting PPU");
    cycles_since_reset = 0;
    frame_cycle = 0;
    //scanline_cycle = 0;
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

	ppu_ctrl = 0x00;
	ppu_mask = 0x00;
	ppu_status = 0x00;
	ppu_scroll = 0x00;
	ppu_data = 0x00;

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

    int dot_modulo = 0; // declare before entering switch
    switch(scanline){

        //case 0 ... 239:
        case -1 ... 239:
            // pre-render line and visible lines

            /*
             * The dummy line, -1 (aka 261) is included in this block since it
             * behaves the same as scanlines 0-239 (loads the tile data into 
             * the shift registers for scanline 0), but doesn't render
             */
            
            switch(dot){
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

                    if(dot == 2){
                        // sprite 0 hit
                    }

                    if(dot >= 9 && (dot % 8) == 1){
                        // reload shift registers every 8 cycles
                    }

                    dot_modulo = ((dot-1) % 8) + 1; // dot mod 8, rolling over to 1 instead of 0
                    switch(dot_modulo){ 
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
                            VNES_LOG::LOG(VNES_LOG::ERROR, "PPU reached bad memory fetch: tried to do memory fetch for cycle %d: cycle %d, scanline %d", dot, dot, scanline);
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
                    VNES_LOG::LOG(VNES_LOG::ERROR, "PPU reached bad dot: dot %d, scanline %d", dot, scanline);
                    break;
            }

            if(odd_frame && scanline == -1 && dot == 339){
                dot++; // scanline -1 skips dot 340 on odd frames and jumps to scanline 0, dot 0
            }
            
            break;

        case 240:
            // post-render line, idle scanline
            break;

        case 241 ... 260:
            // vertical blanking lines

            if(scanline == 241 && dot == 1){
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

    dot++;
    if(dot > 340){
        scanline++;
        dot = 0;
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


