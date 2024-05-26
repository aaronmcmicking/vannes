#pragma once

#include "Mapper.hpp"
#include "../common/log.hpp"
#include "../common/nes_assert.hpp"
#include <vector>

class Mapper000 : public Mapper{
    public:
        Mapper000(std::vector<uint8_t>& prg_rom, std::vector<uint8_t>& chr_rom): 
            prg_rom {prg_rom}, chr_rom {chr_rom}
        { name = "Mapper000"; }

        uint8_t read(uint16_t addr) override {
            using namespace VNES_LOG;

            switch(addr){
                case 0x4020 ... 0x7FFF:
                    return chr_rom[addr % 0x4020];
                    break;
                case 0x8000 ... 0xFFFF:
                    return prg_rom[addr % 0x8000];
                    break;
                default:
                    LOG(ERROR, "Mapper cannot honour cartridge memory read at out-of-bounds address 0x%x (expected range is 0x4020 to 0xFFFF). Returning 0", addr);
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
                    LOG(ERROR, "Mapper cannot write to ROM address 0x%x. Write call has no effect. Data was 0x%x", addr, data);
                    break;
                default:
                    LOG(ERROR, "Mapper cannot write to cartridge memory at out-of-bounds 0x%x (expected range is 0x4020 to 0x8000)", addr);
                    break;
            }
        }

    private:
        // provided by constructor
        std::vector<uint8_t>& prg_rom;
        std::vector<uint8_t>& chr_rom;
};
