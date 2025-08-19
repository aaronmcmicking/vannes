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

            std::fill(std::begin(prg_ram), std::end(prg_ram), 0);

            VNES_LOG::LOG(VNES_LOG::DEBUG, "Done initializing Mapper000");
        }

        uint8_t read(uint16_t addr) override {
            using namespace VNES_LOG;

            switch(addr){
                case 0x6000 ... 0x7FFF:
                    return prg_ram[addr % 0x6000];
                    break;
                case 0x8000 ... 0xFFFF:
                    if(mirror_prg_16kb){
                        return prg_rom[(addr % 0x8000) % 0x4000];
                    }else{
                        return prg_rom[addr % 0x8000];
                    }
                    break;
                default:
                    LOG(ERROR, "Out of bound cartridge mapper read at address 0x%x (expected 0x6000 to 0xFFFF)", addr);
                    return 0;
                    break;
            }
        }

        void write(uint16_t addr, uint8_t data) override {
            using namespace VNES_LOG;
            switch(addr){
                case 0x6000 ... 0x7FFF:
                    prg_ram[addr % 0x6000] = data;
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
                    LOG(ERROR, "Out of bound cartridge mapper write at address 0x%x (expected 0x6000 to 0xFFFF) with data 0x%x", addr, data);
                    break;
            }
        }

    private:
        // provided by constructor
        std::vector<uint8_t>& prg_rom;
        std::vector<uint8_t>& chr_rom;
        uint8_t prg_ram[0x2000] {}; // Original hardware Mapper000 doesn't contain this memory, but some emulators included it,
                                    // so for compatibility 8KiB is included just in case

        // if the program data loaded from the cart is less than 16kb, then 
        // address 0xc000-0xFFFF mirrors 0x8000-0xBFFF
        bool mirror_prg_16kb = false;
};

