#pragma once

#include <exception>
#include "../common/log.hpp"
#include "../common/nes_assert.hpp"
#include <fstream>
#include <inttypes.h>
#include <iostream>
#include "header.hpp"


// dummy header
Header::Header(){
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Constructing dummy header");

    data.nes_title[0] 			    = 0x4E;
    data.nes_title[1] 			    = 0x45;
    data.nes_title[2] 			    = 0x53;
    data.nes_title[3] 			    = 0x1A;
    data.prg_rom_size_lsb 		    = 1;
    data.chr_rom_size_lsb 		    = 1;
    data.flags_6 				    = 0;
    data.flags_7 				    = 0;
    data.mapper_msb_submapper 	    = 0;
    data.prg_chr_rom_msb 		    = 0;
    data.prg_ram_eeprom_shift 	    = 0;
    data.chr_ram_shift 				= 0;
    data.cpu_ppu_timing 			= 0;
    data.hardware_type 				= 0;
    data.misc_roms_present 			= 0;
    data.default_expansion_device   = 0;

    header_format = iNES;

    VNES_LOG::LOG(VNES_LOG::DEBUG, "Finished constructing dummy header");
}

Header::Header(std::ifstream& file){
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Constructing header");
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
        data.prg_ram_eeprom_shift 	    = file.get();
        data.chr_ram_shift 				= file.get();
        data.cpu_ppu_timing 			= file.get();
        data.hardware_type 				= file.get();
        data.misc_roms_present 			= file.get();
        data.default_expansion_device   = file.get();
    }catch(std::exception& e){
        VNES_LOG::LOG(VNES_LOG::FATAL, "Failed to read ROM header with exception: %s", e.what());
        VNES_ASSERT(0 && "Failed to read ROM header");        
    }

    //std::cout << data.nes_title[0] 			    << std::endl;
    //std::cout << data.nes_title[1] 			    << std::endl;
    //std::cout << data.nes_title[2] 			    << std::endl;
    //std::cout << data.nes_title[3] 			    << std::endl;
    //std::cout << std::bitset<8>(data.prg_rom_size_lsb 		    ) << std::endl;
    //std::cout << std::bitset<8>(data.chr_rom_size_lsb 		    ) << std::endl;
    //std::cout << std::bitset<8>(data.flags_6 				    ) << std::endl;
    //std::cout << std::bitset<8>(data.flags_7 				    ) << std::endl;
    //std::cout << std::bitset<8>(data.mapper_msb_submapper 	    ) << std::endl;
    //std::cout << std::bitset<8>(data.prg_chr_rom_msb 		    ) << std::endl;
    //std::cout << std::bitset<8>(data.prg_ram_eeprom_shift 	    ) << std::endl;
    //std::cout << std::bitset<8>(data.chr_ram_shift 				) << std::endl;
    //std::cout << std::bitset<8>(data.cpu_ppu_timing 			) << std::endl;
    //std::cout << std::bitset<8>(data.hardware_type 				) << std::endl;
    //std::cout << std::bitset<8>(data.misc_roms_present 			) << std::endl;
    //std::cout << std::bitset<8>(data.default_expansion_device   ) << std::endl;

    if( !(data.nes_title[0] == 0x4E  // 'N' 
                && data.nes_title[1] == 0x45  // 'E'
                && data.nes_title[2] == 0x53  // 'S'
                && data.nes_title[3] == 0x1A) // MS DOS end-of-line
      ){
        VNES_LOG::LOG(VNES_LOG::ERROR, "Didn't find 'NES' at start of ROM file as expected by iNES standard. Instead got %x %x %x %x. Is the ROM malformed? Continuing anyway, but this may be a bad ROM file!", data.nes_title[0], data.nes_title[1], data.nes_title[2], data.nes_title[3]);
    }

    bool ines2 = ((data.flags_7 & 0x0C) == 0x08);
    if(ines2){
        header_format = iNES2;
    }else{
        header_format = iNES;
    }

    VNES_LOG::LOG(VNES_LOG::DEBUG, "Finished constructing header");
}

