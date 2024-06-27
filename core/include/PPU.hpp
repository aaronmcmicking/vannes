#pragma once

#include "RAM.hpp"
#include "../../common/typedefs.hpp"

class PPU{
    public:
        PPU(RAM& _ram);

    public:
        void power_up();
        void reset();
        void do_cycles(int cycles_to_do);

        void    write(uint16_t addr, uint8_t data);
        uint8_t read(uint16_t addr);

        bool check_nmi();

    private:
        // see https://8bitworkshop.com/blog/platforms/nintendo-nes.md.html for
        // bit-values in specific registers
        enum PPU_regs{
            PPU_CTRL 	    = 0x2000,  // W 	PPU Control 1
            PPU_MASK 	    = 0x2001,  // W 	PPU Control 2
            PPU_STATUS 	    = 0x2002,  // R 	PPU Status
            PPU_OAM_ADDR 	= 0x2003,  // W 	OAM Address
            PPU_OAM_DATA 	= 0x2004,  // R/W 	OAM Data
            PPU_SCROLL 	    = 0x2005,  // W 2x 	Background Scroll Position \newline (write X then Y)
            PPU_ADDR 	    = 0x2006,  // W 2x 	PPU Address \newline (write upper then lower)
            PPU_DATA 	    = 0x2007,  // R/W 	PPU Data
            PPU_OAM_DMA     = 0x4014   // W 	Sprite Page DMA Transfer 
        };

        RAM& ram;
        
        uint64_t cycles_since_reset;
        int frame_cycles;
        int scanline_cycles;
        int scanline;
        int dot;

        //bool vblank;
        bool frame_done;
        bool odd_frame;
        int buffer[256][224];

        //bit nmi_occured;
        //bit nmi_output;

        void    internal_write(uint16_t addr, uint8_t data);
        uint8_t internal_read(uint16_t addr);


};
