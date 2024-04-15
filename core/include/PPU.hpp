#pragma once

class PPU{
    public:
        // see https://8bitworkshop.com/blog/platforms/nintendo-nes.md.html for
        // bit-values in specific registers
        enum PPU_regs{
            PPU_CTRL 	= 0x2000,  // W 	PPU Control 1
            PPU_MASK 	= 0x2001,  // W 	PPU Control 2
            PPU_STATUS 	= 0x2002,  // R 	PPU Status
            OAM_ADDR 	= 0x2003,  // W 	OAM Address
            OAM_DATA 	= 0x2004,  // R/W 	OAM Data
            PPU_SCROLL 	= 0x2005,  // W 2x 	Background Scroll Position \newline (write X then Y)
            PPU_ADDR 	= 0x2006,  // W 2x 	PPU Address \newline (write upper then lower)
            PPU_DATA 	= 0x2007,  // R/W 	PPU Data
            OAM_DMA     = 0x4014   // W 	Sprite Page DMA Transfer 
        };

        void do_cycle();

    private:
};
