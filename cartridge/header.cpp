#include <exception>
#include "../common/log.hpp"
#include <fstream>
#include <inttypes.h>

// Data always assumed to be iNES2 format, since iNES2 is backwards compatible
// with iNES. If iNES format is found, bytes 4, 5, 8, 9 are interpreted slightly
// differently, byte 10 is ignored, and bytes 11-15 are 0 (padding)
struct Header{
    typedef enum HeaderFormat{
        iNES = 1,
        iNES2
    }HeaderFormat;
    
    HeaderFormat header_format;

    struct Data{
        uint8_t nes_title[4];               // -usually 'NES' + DOS null terminator
        uint8_t prg_rom_size_lsb;           // -PRG ROM size LSB [7:0]
        uint8_t chr_rom_size_lsb;           // -CHR ROM size LSB [7:0]
        uint8_t flags_6;                    // -Mapper low nybble [7:4], alt nametable y/n [3], trainer present [2], battery present [1], nametable arrangement [0]
        uint8_t flags_7;                    // -Mapper middle nybble [7:4], if bits[3:2] = 2 then remaining are NES2.0 spec, console type (0: NES, 1: Vs. system, 2: Playchoice 10, 3: extended)
        uint8_t mapper_msb_submapper;       // -Submapper # [7:4], Mapper high nybble [3:0] 
        uint8_t prg_chr_rom_msb;            // -CHR-ROM size MSB [7:4], PRG-ROM size [3:0]
        uint8_t prg_ram_eeprom_sift;        // -PRG-NVRAM/EEPROM (non-volatile) shift count [7:4], PRG-RAM (volatile) shift count [3:0]
                                                // If the shift count is zero, there is no PRG-(NV)RAM.
                                                // If the shift count is non-zero, the actual size is
                                                // "64 << shift count" bytes, i.e. 8192 bytes for a shift count of 7.
        uint8_t chr_ram_size;               // -CHR-NVRAM size (non-volatile) shift count [7:4], CHR-RAM (volatile) shift count [3:0]
                                                // Same shift meaning as above
        uint8_t cpu_ppu_timing;             // -[1:0] CPU/PPU timing mode (0: RP2C02 (NTSC), 1: RP2C07 (Licenced PAL  NES), 2: Multiple region, 3: UA6538 (Dendy), [7:2] unused
        uint8_t hardware_type;              // -System type
                                                // If VS. System type (byte 7): Vs. hardware type [7:4], Vs. PPU type [3:0]
                                                // If Extended console (byte 7): [3:0] Extended console type, [7:4] unused
        uint8_t misc_roms_present;          // -[1:0] Number of misc ROMs present, [7:2] unused
        uint8_t default_expansion_device;   // -[5:0] Default expansion device, [7:6] unused
    };

    struct Data data;

    Header(std::ifstream& file){
        using namespace std;
        file.exceptions(iostream::failbit | iostream::badbit | iostream::eofbit);
        try{
            data.nes_title[0] 			    = file.get();
            data.nes_title[1] 			    = file.get();
            data.nes_title[2] 			    = file.get();
            data.nes_title[3] 			    = file.get();
            data.prg_rom_size_lsb 		    = file.get();
            data.chr_rom_size_lsb 		    = file.get();
            data.flags_6 				    = file.get();
            data.flags_7 				    = file.get();
            data.mapper_msb_submapper 	    = file.get();
            data.prg_chr_rom_msb 		    = file.get();
            data.prg_ram_eeprom_sift 	    = file.get();
            data.chr_ram_size 				= file.get();
            data.cpu_ppu_timing 			= file.get();
            data.hardware_type 				= file.get();
            data.misc_roms_present 			= file.get();
            data.default_expansion_device   = file.get();
        }catch(std::exception e){
            VNES_LOG::LOG(VNES_LOG::FATAL, "Failed to read ROM header with exception: %s", e.what());
        }

        bool ines2 = ((data.flags_7 & 0x0C) == 0x08);
        if(ines2){
            header_format = iNES2;
        }else{
            header_format = iNES;
        }

    }

};
