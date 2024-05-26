#pragma once

#include "Mapper.hpp"
#include "../common/log.hpp"
#include "../common/nes_assert.hpp"

class Mapper000 : public Mapper{
    //uint16_t map(uint16_t addr) override {
    //    switch(addr){
    //        case 0x0000 ... 0x401F:
    //            //VNES_LOG::LOG(VNES_LOG::FATAL, "Address 0x%h is below 0x4020 and cannot be mapped by mapper", addr);
    //            //VNES_ASSERT(0 && "Mapper couldn't map out of range address");
    //            return addr;
    //        case 0x4020 ... 0xFFFF:
    //            if(addr < 0x6000){
    //                return addr % 4020;
    //            }else if(addr < 0x8000){
    //                return addr % 0x6000;
    //            }else{
    //                return addr % 0x8000;
    //            }
    //        default:
    //            VNES_LOG::LOG(VNES_LOG::FATAL, "Unreachable??? Address 0x%h was not covered by mapper", addr);
    //            VNES_ASSERT(0 && "Unreachable");
    //    }
    //}
    public:
        Mapper000(){ name = "Mapper000"; }

        uint8_t read(uint16_t addr) override {
            using namespace VNES_LOG;
            LOG(WARN, "Mapper000.read(addr) called but has no implementation, returning 0");
            (void) addr;
            return 0;
        }

        void write(uint16_t addr, uint8_t data) override {
            using namespace VNES_LOG;
            (void) addr;
            (void) data;
            LOG(WARN, "Mapper000.write(addr, data) called but has no implementation (no effect)");
        }
};
