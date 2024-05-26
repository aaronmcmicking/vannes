#pragma once

#include <string>

class Mapper{
    public:
        std::string name;
        virtual uint8_t read(uint16_t addr) = 0;
        virtual void    write(uint16_t addr, uint8_t data) = 0; // some mappers may reject calls to this
};
