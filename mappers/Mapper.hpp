#pragma once

#include <string>
#include "../common/nes_assert.hpp"

class Mapper{
    public:
        std::string name = "MapperBaseClass";
        virtual uint8_t read(uint16_t addr){ (void)addr; VNES_ASSERT(0 && "Mapper base class should never be used directly"); return 0; };
        virtual void    write(uint16_t addr, uint8_t data) { (void)addr; (void)data; VNES_ASSERT(0 && "Mapper base class should never be used directly"); };
};
