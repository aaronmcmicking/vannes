#pragma once

#include "Mapper.hpp"
#include "../common/log.hpp"
#include "../common/nes_assert.hpp"
#include <vector>

class Mapper000 : public Mapper{
    public:
        Mapper000(std::vector<uint8_t>& prg_rom, std::vector<uint8_t>& chr_rom): 
            prg_rom {prg_rom}, chr_rom {chr_rom}
        { 
            VNES_LOG::LOG(VNES_LOG::DEBUG, "Initializing Mapper000");
            name = "Mapper000"; 
            if(prg_rom.size() <= 16384){
                mirror_prg_16kb = true;
                VNES_LOG::LOG(VNES_LOG::DEBUG, "Mapper sees that PRG ROM is less than 16KiB and will mirror PRG-ROM 0xC000-0xFFFF as 0x8000-0xBFFF");
            }else{
                mirror_prg_16kb = false;
                VNES_LOG::LOG(VNES_LOG::DEBUG, "Mapper sees that PRG ROM is more than 16KiB and will not mirror PRG-ROM");
            }
            VNES_LOG::LOG(VNES_LOG::DEBUG, "Done initializing Mapper000");
        }

        uint8_t read(uint16_t addr) override {
            using namespace VNES_LOG;

            switch(addr){
                case 0x4020 ... 0x7FFF:
                    LOG(FATAL, "CHR ROM is not accessible by the CPU and should only be accessed through the PPU (pattern table 0x0000 - 0x1FFF)");
                    return chr_rom[addr % 0x4020];
                    break;
                case 0x8000 ... 0xFFFF:
                    if(mirror_prg_16kb){
                        return prg_rom[(addr % 0x8000) % 0x4000];
                    }else{
                        return prg_rom[addr % 0x8000];
                    }
                    break;
                default:
                    LOG(ERROR, "Mapper cannot read from cartridge memory at out-of-bounds address 0x%x (expected range is 0x4020 to 0xFFFF). Returning 0", addr);
                    return 0;
                    break;
            }
        }

        void write(uint16_t addr, uint8_t data) override {
            using namespace VNES_LOG;
            switch(addr){
                case 0x4020 ... 0x7FFF:
                    chr_rom[addr % 0x4020] = data;
                    break;
                case 0x8000 ... 0xFFFF:
                    LOG(WARN, "Mapper write to ROM address 0x%x. Write will be allowed as it may be for debug purposes. Data is 0x%x", addr, data);
                    if(mirror_prg_16kb){
                        prg_rom[(addr % 0x8000) % 0x4000] = data;
                    }else{
                        prg_rom[addr % 0x8000] = data;
                    }
                    break;
                default:
                    LOG(ERROR, "Mapper cannot write to cartridge memory at out-of-bounds address 0x%x (expected range is 0x4020 to 0x8000)", addr);
                    break;
            }
        }

    private:
        // provided by constructor
        std::vector<uint8_t>& prg_rom;
        std::vector<uint8_t>& chr_rom;

        // if the program data loaded from the cart is less than 16kb, then 
        // address 0xc000-0xFFFF mirrors 0x8000-0xBFFF
        bool mirror_prg_16kb = false;
};
