#pragma once

//#include "RAM.hpp"
#include "../../cartridge/cartridge.hpp"
#include "DMABus.hpp"
#include "../../common/typedefs.hpp"

class PPU{
    public:
        //PPU(RAM& _ram, Cartridge& _cart);
        PPU(Cartridge& _cart, DMABus& _dmabus);
        void power_up();
        void reset();
        void do_cycles(int cycles_to_do);

        void register_write(uint16_t addr, uint8_t data);
        uint8_t register_read(uint16_t addr);

        //void    write(uint16_t addr, uint8_t data);
        //uint8_t read(uint16_t addr);

        bool check_nmi();

        //int buffer[256][224]; // x = 256, y = 244, so index as buffer[x][y]
        int buffer[256*224]; 

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

    private:
        /*
         * TODO remove me
          good memory map resource https://forums.nesdev.org/viewtopic.php?t=10297
         */
        
        // internal ram
        /*
         * The VRAM address space contains graphics data, including pattern tables,
         * nametables, attribute tables, and pallete data. Most of the address
         * space is mapped by the cartridge, meaning that reads/writes to these
         * areas will pass through the cartridge mapper. The mapper may route the
         * R/W to on-cartridge ROM/RAM, such as CHR ROM/RAM, or it may reflect
         * the R/W to the PPU's internal VRAM.
         *
         * From https://www.nesdev.org/wiki/PPU_memory_map:
         *  $0000-1FFF is normally mapped by the cartridge to a CHR-ROM or CHR-RAM, often with a bank switching mechanism.
         *  $2000-2FFF is normally mapped to the 2kB NES internal VRAM, providing 2 nametables with a mirroring configuration controlled by the cartridge, but it can be partly or fully remapped to ROM or RAM on the cartridge, allowing up to 4 simultaneous nametables.
         *  $3000-3EFF is usually a mirror of the 2kB region from $2000-2EFF. The PPU does not render from this address range, so this space has negligible utility.
         *  $3F00-3FFF is not configurable, always mapped to the internal palette control.
         *
         *
         * Table from https://www.nesdev.org/wiki/PPU_memory_map
         * Range                Size    Function                Mapped by
         * 0x0000 - 0x0FFF:     0x1000 	Pattern table 0 	    Cartridge
         * 0x1000 - 0x1FFF: 	0x1000 	Pattern table 1 	    Cartridge
         * 0x2000 - 0x23BF: 	0x03c0 	Nametable 0 	        Cartridge
         * 0x23C0 - 0x23FF: 	0x0040 	Attribute table 0 	    Cartridge
         * 0x2400 - 0x27BF: 	0x03c0 	Nametable 1 	        Cartridge
         * 0x27C0 - 0x27FF: 	0x0040 	Attribute table 1 	    Cartridge
         * 0x2800 - 0x2BBF: 	0x03c0 	Nametable 2 	        Cartridge
         * 0x2BC0 - 0x2BFF: 	0x0040 	Attribute table 2 	    Cartridge
         * 0x2C00 - 0x2FBF: 	0x03c0 	Nametable 3 	        Cartridge
         * 0x2FC0 - 0x2FFF: 	0x0040 	Attribute table 3 	    Cartridge
         * 0x3000 - 0x3EFF: 	0x0F00 	Unused 	                Cartridge
         * 0x3F00 - 0x3F1F: 	0x0020 	Palette RAM indexes 	Internal to PPU
         * 0x3F20 - 0x3FFF: 	0x00E0 	Mirrors of $3F00-$3F1F 	Internal to PPU
         */
        uint8_t vram[0x4000]; // 0x4000 = 16384 bytes = 14-bit address line
                                 // see https://www.nesdev.org/wiki/PPU_memory_map
                                 //

        // registers
        uint8_t ppu_ctrl;
        uint8_t ppu_mask;
        uint8_t ppu_status;
        uint8_t ppu_oam_addr;
        uint8_t ppu_oam_data;
        uint16_t ppu_scroll;
        uint16_t ppu_addr;
        uint8_t ppu_data;
        uint8_t ppu_oam_dma;

        //  VRAM reads are too slow to keep up the the CPU, so when data is read from PPU_DATA, the previous value is returned and the newly fetched value is stored in the buffer. This causes the byte read from PPU_DATA to be 'delayed' by one read.
        // see https://www.nesdev.org/wiki/PPU_registers#The_PPUDATA_read_buffer_(post-fetch)
        uint8_t ppu_data_read_buffer;

        // OAM
        uint8_t OAM_PRIMARY[4*64]; // primary OAM can hold 64 sprites, each 4 bytes long
        uint8_t OAM_SECONDARY[4*8]; // secondary OAM holds only 8 sprites

        // internal registers, see https://www.nesdev.org/wiki/PPU_scrolling
        uint16_t reg_v; // 15 bits, current VRAM address. Note PPU address is 14 bits wide, so top bit unused through 0x2007
        uint16_t reg_t; // 15 bits, temp VRAM address (also thought as address of top left onscreen tile)
        uint8_t reg_x; // 3 bits, fine X scroll
        uint8_t reg_w; // 1 bit, first or second write toggle

        //RAM& ram;
        DMABus& dmabus;
        Cartridge& cart;
        
        uint64_t cycles_since_reset;
        int frame_cycle;
        //int scanline_cycle;
        int scanline;
        int dot;

        //bool vblank;
        bool frame_done;
        bool odd_frame;

        //bit nmi_occured;
        //bit nmi_output;

        //void    internal_write(uint16_t addr, uint8_t data);
        //uint8_t internal_read(uint16_t addr);

        void cycle();


};
