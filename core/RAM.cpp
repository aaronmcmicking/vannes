#pragma once

#include "include/RAM.hpp"
#include "../common/nes_assert.hpp"
#include "../common/log.hpp"

RAM::RAM(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing RAM...");
}

void RAM::write(uint16_t addr, uint8_t data){
    mem[addr] = data;
}

uint8_t RAM::read(uint16_t addr){
    return mem[addr];
}

void RAM::write_nmi_vec(uint16_t data){
	VNES_LOG::LOG(VNES_LOG::WARN, "write_nmi_vec(): Check that I'm implemented correctly!");
    mem[NMI_VEC]     = (uint8_t)data; // LB
    mem[NMI_VEC + 1] = (uint8_t)(data >> 8); // HB
}

void RAM::write_reset_vec(uint16_t data){
	VNES_LOG::LOG(VNES_LOG::WARN, "write_reset_vec(): Check that I'm implemented correctly!");
    mem[RESET_VEC] =  (uint8_t)data; // LB
    mem[RESET_VEC + 1] =  (uint8_t)(data >> 8); // HB
}

void RAM::write_brk_vec(uint16_t data){
	VNES_LOG::LOG(VNES_LOG::WARN, "write_brk_vec(): Check that I'm implemented correctly!");
    mem[BRK_VEC]     = (uint8_t)data; // LB
    mem[BRK_VEC + 1] = (uint8_t)(data >> 8); // HB
}

