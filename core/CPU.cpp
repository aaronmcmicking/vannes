#pragma once

#include <iostream>
#include <sstream>
#include <algorithm>
#include "include/CPU.hpp"
#include "../common/nes_assert.hpp"
#include "../common/log.hpp"

CPU::CPU(RAM& _ram, PPU& _ppu): ram {_ram}, ppu {_ppu}{
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing CPU...");
    power_up();
    //printf("after powerup, PC is (decimal) %u\n", program_counter);
}

void CPU::step(){
    using namespace VNES_LOG;

    //if(ppu.check_nmi()){ // start of vblank
    //    raise_interrupt(false);
    //}

    uint8_t opcode = fetch_instruction();
    LOG(DEBUG, "Fetched opcode 0x%x from address 0x%x", opcode, program_counter);
    uint64_t cycles_before = frame_cycles;
    execute_instruction(opcode);
    int cycles_done = frame_cycles - cycles_before; 
    cycles_since_reset += cycles_done;
    LOG(DEBUG, "Opcode consumed %d cycles (%lld this frame)", cycles_done, frame_cycles);
    program_counter++;

    ppu.do_cycles(cycles_done*3);
    LOG(INFO, "%2x  A:%2x X:%2x Y:%2x SP:%2x\n", program_counter, accumulator, index_X, index_Y, stack_pointer);
}

inline uint8_t CPU::fetch_instruction(){
    return read_mem(program_counter);
}

inline uint8_t CPU::read_mem(uint16_t addr){
    return ram.read(addr);
}

inline void CPU::write_mem(uint16_t addr, uint8_t data){
    ram.write(addr, data);
}

inline void CPU::push_stack(uint8_t data){
    write_mem(stack_pointer--, data);
}

inline uint8_t CPU::pop_stack(){
    return read_mem(++stack_pointer);
}

void CPU::write_nmi_vec(uint16_t data){
	VNES_LOG::LOG(VNES_LOG::WARN, "write_nmi_vec(): Check that I'm implemented correctly!");
    write_mem(RAM::VEC_ADDR::NMI_VEC, (uint8_t)data); // LB
    write_mem(RAM::VEC_ADDR::NMI_VEC + 1, (uint8_t)(data >> 8)); // HB
}

uint16_t CPU::read_nmi_vec(){
	VNES_LOG::LOG(VNES_LOG::WARN, "read_nmi_vec(): Check that I'm implemented correctly!");
    uint16_t data = 0;
    data |= read_mem(RAM::VEC_ADDR::NMI_VEC + 1); // HB
    data <<= 8;
    data |= read_mem(RAM::VEC_ADDR::NMI_VEC); // LB
    return data;
}

void CPU::write_reset_vec(uint16_t data){
	VNES_LOG::LOG(VNES_LOG::WARN, "write_reset_vec(): Check that I'm implemented correctly!");
    write_mem(RAM::VEC_ADDR::RESET_VEC, (uint8_t)data); // LB
    write_mem(RAM::VEC_ADDR::RESET_VEC + 1, (uint8_t)(data >> 8)); // HB
}

uint16_t CPU::read_reset_vec(){
    uint16_t data = 0;
    data |= read_mem(RAM::VEC_ADDR::RESET_VEC + 1); // HB
    data <<= 8;
    data |= read_mem(RAM::VEC_ADDR::RESET_VEC); // LB
    return data;
}

void CPU::write_brk_vec(uint16_t data){
	VNES_LOG::LOG(VNES_LOG::WARN, "write_brk_vec(): Check that I'm implemented correctly!");
    write_mem(RAM::VEC_ADDR::BRK_VEC, (uint8_t)data); // LB
    write_mem(RAM::VEC_ADDR::BRK_VEC + 1, (uint8_t)(data >> 8)); // HB
}

uint16_t CPU::read_brk_vec(){
	VNES_LOG::LOG(VNES_LOG::WARN, "read_brk_vec(): Check that I'm implemented correctly!");
    uint16_t data = 0;
    data |= read_mem(RAM::VEC_ADDR::BRK_VEC + 1); // HB
    data <<= 8;
    data |= read_mem(RAM::VEC_ADDR::BRK_VEC); // LB
    return data;
}

void CPU::power_up(){
    // The power-up sequence consumes 8 cycles in actual hardware: 0 to 
    // assert the interrupt disable flag and 2 to load the reset vector 
    // into the program counter. The first program instruction occurs on
    // the 8th cycles when the start-up sequence releases control. The other
    // 5 cyles consist of 2 NOPs and 3 stack operations, although these
    // have no effect as the processor is in a disabled state during start-up.
	VNES_LOG::LOG(VNES_LOG::INFO, "Entered power-up sequence");
    VNES_LOG::LOG(VNES_LOG::WARN, "power_up(): Check that I'm implemented right!");
    interrupt_disable_f = 1;
    stack_pointer = 0x00; // reset() will decrement to 0xFD
    set_status_reg(0x24);
    //program_counter = read_reset_vec();
    reset();
    VNES_LOG::LOG(VNES_LOG::DEBUG, "After reset, SP is 0x%x and status reg is 0x%x", stack_pointer, status_as_int());
    VNES_LOG::LOG(VNES_LOG::INFO, "Power-up sequence done");
}

void CPU::reset(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Received reset signal");
    program_counter = read_reset_vec();
    VNES_LOG::LOG(VNES_LOG::INFO, "Loaded RESET Vec to PC. PC is now 0x%x.", program_counter);
    stack_pointer -= 3;
    interrupt_disable_f = 1;
    ppu.reset();
}

//void CPU::engage_reset(){
//	VNES_LOG::LOG(VNES_LOG::INFO, "Engaged reset signal");
//	VNES_LOG::LOG(VNES_LOG::ERROR, "Reset signal is not implemented!");
//}
//
//void CPU::release_reset(){
//	VNES_LOG::LOG(VNES_LOG::INFO, "Releasing reset signal");
//	VNES_LOG::LOG(VNES_LOG::ERROR, "Reset signal is not implemented!");
//}

// Interrupt routines differ for maskable (BRK/IRQ) and non-maskable
// interrupts. However, the return-from-interrupt sequence 
// is the same (see: RTI())
void CPU::raise_interrupt(bool maskable){
    auto push_program_counter = [this](){
        push_stack(program_counter >> 8);
        push_stack(program_counter);
    };

    // since I must have been low for the interrupt
    // and P is stored before I is asserted, 
    // I will always be deasserted by RTI
    if(maskable){
        if(CPU::MASKABLE_IRQ && interrupt_disable_f){
            return;
        }
        push_program_counter();
        b_flag_f = 1;
        push_stack(status_as_int());
        program_counter = read_brk_vec();
    }else{
        push_program_counter();
        push_stack(status_as_int());
        program_counter = read_nmi_vec();
    }
    frame_cycles++;
    interrupt_disable_f = 1;
}

uint8_t CPU::status_as_int(){ 
    VNES_LOG::LOG(VNES_LOG::WARN, "status_as_int: Check that I'm implemented right!");
    uint8_t status = 0b00100000; // bit 5 is always 1
    if(negative_f)            status |= 0b10000000;
    if(overflow_f)            status |= 0b01000000;
    if(b_flag_f)              status |= 0b00010000;
    if(decimal_f)             status |= 0b00001000;
    if(interrupt_disable_f)   status |= 0b00000100;
    if(zero_f)                status |= 0b00000010;
    if(carry_f)               status |= 0b00000001;
    return status;
}

void CPU::set_status_reg(uint8_t status){
    negative_f          = (status & 0b10000000);
    overflow_f          = (status & 0b01000000);
    b_flag_f            = (status & 0b00010000);
    decimal_f           = (status & 0b00001000);
    interrupt_disable_f = (status & 0b00000100);
    zero_f              = (status & 0b00000010);
    carry_f             = (status & 0b00000001);
}

void CPU::add_cycle_if_page_crossed(uint16_t base_addr, 
                                    uint16_t offset, 
                                    CPU::ADDRESSING_MODE mode,
                                    const AddrModeVec& modes){
    bool check_in_this_mode = (std::find(modes.begin(), modes.end(), mode) != modes.end());
    if(check_in_this_mode){
        uint16_t result_addr = base_addr + offset;
        base_addr &= 0xF0;
        result_addr &= 0xF0;
        if(base_addr != result_addr){
            // page crossed
            frame_cycles++;
        }
    }
}

// TODO: verify that page wrapping / page crossing is implemented correctly
uint16_t CPU::fetch_address(enum ADDRESSING_MODE mode, const AddrModeVec& page_crossing_modes = AddrModeVec{}){
    using namespace VNES_LOG;
    uint16_t addr = 0;
    uint8_t zpg_ptr = 0; // a pointer into the zero page is sometimes needed
    switch(mode){
        case ACC:
            addr = 0; // The accumulator is not in RAM.
                      // Instructions which work on the accumulator should not attempt to
                      // access RAM
            LOG(ERROR, "Tried to fetch address while in Accumulator mode. This attempt shouldn't happen!\n");
			break;
		case ABS:
            // ABS has low, then high byte of programmer's desired address at PC+1 and PC+2
            addr |= read_mem(++program_counter);
            addr |= ((uint16_t)read_mem(++program_counter) << 8);
			break;
		case ABSX:
            addr |= read_mem(++program_counter);
            addr |= ((uint16_t)read_mem(++program_counter) << 8);
            add_cycle_if_page_crossed(addr, index_X, mode, page_crossing_modes);
            addr += index_X;
			break;
		case ABSY:
            addr |= read_mem(++program_counter);
            addr |= ((uint16_t)read_mem(++program_counter) << 8);
            add_cycle_if_page_crossed(addr, index_Y, mode, page_crossing_modes);
            addr += index_Y;
			break;
		case IMM:
            addr = ++program_counter;
			break;
		case IMPL:
            addr = 0; // Implicit addressing never needs to fetch an address
            LOG(ERROR, "Tried to fetch address while in Implied mode. This attempt shouldn't happen!\n");
			break;
		case IND:
            zpg_ptr = read_mem(++program_counter);
            addr |= read_mem(zpg_ptr);
            addr |= (read_mem(++zpg_ptr) << 8);
			break;
		case INDX:
            zpg_ptr = read_mem(++program_counter);
            zpg_ptr += index_X;
            addr |= read_mem(zpg_ptr);
            addr |= (read_mem(++zpg_ptr) << 8);
			break;
		case INDY:
            zpg_ptr = read_mem(++program_counter);
            add_cycle_if_page_crossed(zpg_ptr, index_Y, mode, page_crossing_modes);
            zpg_ptr += index_Y;
            addr |= read_mem(zpg_ptr);
            addr |= (read_mem(++zpg_ptr) << 8);
			break;
		case REL:
            addr = read_mem(++program_counter);
			break;
		case ZPG:
            addr |= read_mem(++program_counter);
			break;
		case ZPGX:
            addr |= read_mem(++program_counter);
            addr += index_X;
			break;
		case ZPGY:
            addr |= read_mem(++program_counter);
            addr += index_Y;
			break;
        default:
            LOG(FATAL, "Tried to fetch address with bad addressing mode! Bad mode was %c", mode);
            VNES_ASSERT(0 && "Unreachable");
            break;
    }
    return addr;
}

// returns cycles passed
int CPU::execute_instruction(uint8_t instruction){
    std::stringstream ss {};
    ss << (OPCODE)instruction;
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Executing instruction %s", ss.str().c_str());
    switch(instruction){
        /* Load/Store */
        case LDA_INDX:    // Load Accumulator 	N,Z
            LDA(INDX);
            frame_cycles += 6;
            break;
        case LDA_ZPG:    
            LDA(ZPG);
            frame_cycles += 3;
            break;
        case LDA_IMM:    
            LDA(IMM);
            frame_cycles += 2;
            break;
        case LDA_ABS:    
            LDA(ABS);
            frame_cycles += 4;
            break;
        case LDA_INDY:    
            frame_cycles += 5;
            LDA(INDY);
            break;
        case LDA_ZPGX:    
            LDA(ZPGX);
            frame_cycles += 4;
            break;
        case LDA_ABSY:    
            LDA(ABSY);
            frame_cycles += 4;
            break;
        case LDA_ABSX:    
            LDA(ABSX);
            frame_cycles += 4;
            break;
        case LDX_IMM: 	// Load X Register 	    N,Z
            LDX(IMM);
            frame_cycles += 2;
            break;
        case LDX_ZPG: 	
            LDX(ZPG);
            frame_cycles += 3;
            break;
        case LDX_ABS: 	
            LDX(ABS);
            frame_cycles += 4;
            break;
        case LDX_ZPGY: 	
            LDX(ZPGY);
            frame_cycles += 4;
            break;
        case LDX_ABSY: 	
            LDX(ABSY);
            frame_cycles += 4;
            break;
        case LDY_IMM: 	// Load Y Register 	    N,Z
            LDY(IMM);
            frame_cycles += 2;
            break;
        case LDY_ZPG: 	
            LDY(ZPG);
            frame_cycles += 3;
            break;
        case LDY_ABS: 	
            LDY(ABS);
            frame_cycles += 4;
            break;
        case LDY_ZPGX: 	
            LDY(ZPGX);
            frame_cycles += 4;
            break;
        case LDY_ABSX: 	
            LDY(ABSX);
            frame_cycles += 4;
            break;
        case STA_INDX: 	// Store Accumulator 	 
            STA(INDX);
            frame_cycles += 6;
            break;
        case STA_ZPG: 	 	 
            STA(ZPG);
            frame_cycles += 3;
            break;
        case STA_ABS: 	 	 
            STA(ABS);
            frame_cycles += 4;
            break;
        case STA_INDY: 	 	 
            STA(INDY);
            frame_cycles += 6;
            break;
        case STA_ZPGX: 	 	 
            STA(ZPGX);
            frame_cycles += 4;
            break;
        case STA_ABSY: 	 	 
            STA(ABSY);
            frame_cycles += 5;
            break;
        case STA_ABSX: 	 	 
            STA(ABSX);
            frame_cycles += 5;
            break;
        case STX_ZPG: 	// Store X Register 	 
            STX(ZPG);
            frame_cycles += 3;
            break;
        case STX_ABS: 	 	 
            STX(ABS);
            frame_cycles += 4;
            break;
        case STX_ZPGY: 	 	 
            STX(ZPGY);
            frame_cycles += 4;
            break;
        case STY_ZPG: 	// Store Y Register 	 
            STY(ZPG);
            frame_cycles += 3;
            break;
        case STY_ABS: 	 	 
            STY(ABS);
            frame_cycles += 4;
            break;
        case STY_ZPGX: 	 	 
            STY(ZPGX);
            frame_cycles += 4;
            break;

        /* Register Transfers */
        case TAX_IMPL: 	    // Transfer accumulator to X 	N,Z
            TAX();
            frame_cycles += 2;
			break;
        case TAY_IMPL: 	    // Transfer accumulator to Y 	N,Z
            TAY();
            frame_cycles += 2;
			break;
        case TXA_IMPL: 	    // Transfer X to accumulator 	N,Z
            TXA();
            frame_cycles += 2;
			break;
        case TYA_IMPL: 	    // Transfer Y to accumulator 	N,Z
            TYA();
            frame_cycles += 2;
            break;

        /* Stack Operations */
        case TSX_IMPL: 	    // Transfer stack pointer to X 	    N,Z
            TSX();
            frame_cycles += 2;
			break;
        case TXS_IMPL: 	    // Transfer X to stack pointer 	 
            TXS();
            frame_cycles += 2;
			break;
        case PHA_IMPL: 	    // Push accumulator on stack 	 
            PHA();
            frame_cycles += 3;
			break;
        case PHP_IMPL: 	    // Push processor status on stack 	 
            PHP();
            frame_cycles += 3;
			break;
        case PLA_IMPL: 	    // Pull accumulator from stack 	    N,Z
            PLA();
            frame_cycles += 4;
			break;
        case PLP_IMPL: 	    // Pull processor status from stack All
            PLP();
            frame_cycles += 4;
            break;

        /* Logical */
        case AND_INDX: 	    // Logical AND 	            N,Z
            AND(INDX);
            frame_cycles += 6;
            break;
        case AND_ZPG: 	    
            AND(ZPG);
            frame_cycles += 3;
            break;
        case AND_IMM: 	    
            AND(IMM);
            frame_cycles += 2;
            break;
        case AND_ABS: 	    
            AND(ABS);
            frame_cycles += 4;
            break;
        case AND_INDY: 	    
            AND(INDY);
            frame_cycles += 5;
            break;
        case AND_ZPGX: 	    
            AND(ZPGX);
            frame_cycles += 4;
            break;
        case AND_ABSY: 	    
            AND(ABSY);
            frame_cycles += 4;
            break;
        case AND_ABSX: 	    
            AND(ABSX);
            frame_cycles += 4;
            break;
        case EOR_INDX: 	    // Exclusive OR     N,Z
            EOR(INDX);
            frame_cycles += 6;
            break;
        case EOR_ZPG: 	    
            EOR(ZPG);
            frame_cycles += 3;
            break;
        case EOR_IMM: 	    
            EOR(IMM);
            frame_cycles += 2;
            break;
        case EOR_ABS: 	    
            EOR(ABS);
            frame_cycles += 4;
            break;
        case EOR_INDY: 	    
            EOR(INDY);
            frame_cycles += 5;
            break;
        case EOR_ZPGX: 	    
            EOR(ZPGX);
            frame_cycles += 4;
            break;
        case EOR_ABSY: 	    
            frame_cycles += 4;
            EOR(ABSY);
            break;
        case EOR_ABSX: 	    
            EOR(ABSX);
            frame_cycles += 4;
            break;
        case ORA_INDX: 	    // Logical Inclusive OR 	N,Z
            ORA(INDX);
            frame_cycles += 6;
            break;
        case ORA_ZPG: 	    
            ORA(ZPG);
            frame_cycles += 3;
            break;
        case ORA_IMM: 	    
            ORA(IMM);
            frame_cycles += 2;
            break;
        case ORA_ABS: 	    
            ORA(ABS);
            frame_cycles += 4;
            break;
        case ORA_INDY: 	    
            ORA(INDY);
            frame_cycles += 5;
            break;
        case ORA_ZPGX: 	    
            ORA(ZPGX);
            frame_cycles += 4;
            break;
        case ORA_ABSY: 	    
            ORA(ABSY);
            frame_cycles += 4;
            break;
        case ORA_ABSX: 	    
            ORA(ABSX);
            frame_cycles += 4;
            break;
        case BIT_ZPG: 	    // Bit Test 	            N,V,Z
            BIT(ZPG);
            frame_cycles += 3;
            break;
        case BIT_ABS: 	    
            BIT(ABS);
            frame_cycles += 4;
            break;

        /* Arithmetic */
        case ADC_INDX: 	    // Add with Carry 	    N,V,Z,C 
            ADC(INDX);
            frame_cycles += 6;
            break;
        case ADC_ZPG: 	    
            ADC(ZPG);
            frame_cycles += 3;
            break;
        case ADC_IMM: 	    
            ADC(IMM);
            frame_cycles += 2;
            break;
        case ADC_ABS: 	    
            ADC(ABS);
            frame_cycles += 4;
            break;
        case ADC_INDY: 	    
            ADC(INDY);
            frame_cycles += 5;
            break;
        case ADC_ZPGX: 	    
            ADC(ZPGX);
            frame_cycles += 4;
            break;
        case ADC_ABSY: 	    
            ADC(ABSY);
            frame_cycles += 4;
            break;
        case ADC_ABSX: 	    
            ADC(ABSX);
            frame_cycles += 4;
            break;
        case SBC_INDX: 	    // Subtract with Carry 	N,V,Z,C
            SBC(INDX);
            frame_cycles += 6;
            break;
        case SBC_ZPG: 	    
            SBC(ZPG);
            frame_cycles += 3;
            break;
        case SBC_IMM: 	    
            SBC(IMM);
            frame_cycles += 2;
            break;
        case SBC_ABS: 	    
            SBC(ABS);
            frame_cycles += 4;
            break;
        case SBC_INDY: 	    
            SBC(INDY);
            frame_cycles += 5;
            break;
        case SBC_ZPGX: 	    
            SBC(ZPGX);
            frame_cycles += 4;
            break;
        case SBC_ABSY: 	    
            SBC(ABSY);
            frame_cycles += 4;
            break;
        case SBC_ABSX: 	    
            SBC(ABSX);
            frame_cycles += 4;
            break;
        case CMP_INDX: 	    // Compare accumulator 	N,Z,C
            CMP(INDX);
            frame_cycles += 6;
            break;
        case CMP_ZPG: 	    
            CMP(ZPG);
            frame_cycles += 3;
            break;
        case CMP_IMM: 	    
            CMP(IMM);
            frame_cycles += 2;
            break;
        case CMP_ABS: 	    
            CMP(ABS);
            frame_cycles += 4;
            break;
        case CMP_INDY: 	    
            CMP(INDY);
            frame_cycles += 5;
            break;
        case CMP_ZPGX: 	    
            CMP(ZPGX);
            frame_cycles += 4;
            break;
        case CMP_ABSY: 	    
            CMP(ABSY);
            frame_cycles += 4;
            break;
        case CMP_ABSX: 	    
            CMP(ABSX);
            frame_cycles += 4;
            break;
        case CPX_IMM: 	    // Compare X register 	N,Z,C
            CPX(IMM);
            frame_cycles += 2;
            break;
        case CPX_ZPG: 	    
            CPX(ZPG);
            frame_cycles += 3;
            break;
        case CPX_ABS: 	    
            CPX(ABS);
            frame_cycles += 4;
            break;
        case CPY_IMM: 	    // Compare Y register 	N,Z,C
            CPY(IMM);
            frame_cycles += 2;
            break;
        case CPY_ZPG: 	    
            CPY(ZPG);
            frame_cycles += 3;
            break;
        case CPY_ABS: 	    
            CPY(ABS);
            frame_cycles += 4;
            break;

        /* Increments & Decrements */
        case INC_ZPG: 	    // Increment a memory location 	N,Z
            INC(ZPG);
            frame_cycles += 5;
            break;
        case INC_ABS: 	    
            INC(ABS);
            frame_cycles += 6;
            break;
        case INC_ZPGX: 	    
            INC(ZPGX);
            frame_cycles += 6;
            break;
        case INC_ABSX: 	    
            INC(ABSX);
            frame_cycles += 7;
            break;
        case INX_IMPL: 	    // Increment the X register 	N,Z
            INX();
            frame_cycles += 2;
            break;
        case INY_IMPL: 	    // Increment the Y register 	N,Z
            INY();
            frame_cycles += 2;
            break;
        case DEC_ZPG: 	    // Decrement a memory location 	N,Z
            DEC(ZPG);
            frame_cycles += 5;
            break;
        case DEC_ABS: 	    
            DEC(ABS);
            frame_cycles += 6;
            break;
        case DEC_ZPGX: 	    
            DEC(ZPGX);
            frame_cycles += 6;
            break;
        case DEC_ABSX: 	    
            DEC(ABSX);
            frame_cycles += 7;
            break;
        case DEX_IMPL: 	    // Decrement the X register 	N,Z
            DEX();
            frame_cycles += 2;
            break;
        case DEY_IMPL: 	    // Decrement the Y register 	N,Z
            DEY();
            frame_cycles += 2;
            break;

        /* Shifts */
        case ASL_ZPG: 	    // Arithmetic Shift Left 	N,Z,C
            ASL(ZPG);
            frame_cycles += 5;
            break;
        case ASL_ACC: 	    
            ASL_eACC();
            frame_cycles += 2;
            break;
        case ASL_ABS: 	    
            ASL(ABS);
            frame_cycles += 6;
            break;
        case ASL_ZPGX: 	    
            ASL(ZPGX);
            frame_cycles += 6;
            break;
        case ASL_ABSX: 	    
            ASL(ABSX);
            frame_cycles += 7;
            break;
        case LSR_ZPG: 	    // Logical Shift Right 	    N,Z,C
            LSR(ZPG);
            frame_cycles += 5;
            break;
        case LSR_ACC: 	    
            LSR_eACC();
            frame_cycles += 2;
            break;
        case LSR_ABS: 	    
            LSR(ABS);
            frame_cycles += 6;
            break;
        case LSR_ZPGX: 	    
            LSR(ZPGX);
            frame_cycles += 6;
            break;
        case LSR_ABSX: 	    
            LSR(ABSX);
            frame_cycles += 7;
            break;
        case ROL_ZPG: 	    // Rotate Left 	            N,Z,C
            ROL(ZPG);
            frame_cycles += 5;
            break;
        case ROL_ACC: 	    
            ROL_eACC();
            frame_cycles += 2;
            break;
        case ROL_ABS: 	    
            ROL(ABS);
            frame_cycles += 6;
            break;
        case ROL_ZPGX: 	    
            ROL(ZPGX);
            frame_cycles += 6;
            break;
        case ROL_ABSX: 	    
            ROL(ABSX);
            frame_cycles += 7;
            break;
        case ROR_ZPG: 	    // Rotate Right 	        N,Z,C
            ROR(ZPG);
            frame_cycles += 5;
            break;
        case ROR_ACC: 	    
            ROR_eACC();
            frame_cycles += 2;
            break;
        case ROR_ABS: 	    
            frame_cycles += 6;
            ROR(ABS);
            break;
        case ROR_ZPGX: 	    
            ROR(ZPGX);
            frame_cycles += 6;
            break;
        case ROR_ABSX: 	    
            ROR(ABSX);
            frame_cycles += 7;
            break;

        /* Jumps & Calls */
        case JMP_ABS: 	    // Jump to another location 	 
            JMP(ABS);
            frame_cycles += 3;
            break;
        case JMP_IND: 	     	 
            JMP(IND);
            frame_cycles += 5;
            break;
        case JSR_ABS: 	    // Jump to a subroutine 	 
            JSR();
            frame_cycles += 6;
            break;
        case RTS_IMPL: 	    // Return from subroutine 	 
            RTS();
            frame_cycles += 6;
            break;

        /* Branches */
        case BCC_REL: 	    // Branch if carry flag clear 	 
            BCC();
            frame_cycles += 2;
			break;
        case BCS_REL: 	    // Branch if carry flag set 	 
            BCS();
            frame_cycles += 2;
			break;
        case BEQ_REL: 	    // Branch if zero flag set 	 
            BEQ();
            frame_cycles += 2;
			break;
        case BMI_REL: 	    // Branch if negative flag set 	 
            BMI();
            frame_cycles += 2;
			break;
        case BNE_REL: 	    // Branch if zero flag clear 	 
            BNE();
            frame_cycles += 2;
			break;
        case BPL_REL: 	    // Branch if negative flag clear 	 
            BPL();
            frame_cycles += 2;
			break;
        case BVC_REL: 	    // Branch if overflow flag clear 	 
            BVC();
            frame_cycles += 2;
			break;
        case BVS_REL: 	    // Branch if overflow flag set 	 
            BVS();
            frame_cycles += 2;
			break;

        /* Status Flag Changes */
        case CLC_IMPL: 	    // Clear carry flag 	            C
            CLC();
            frame_cycles += 2;
			break;
        case CLD_IMPL: 	    // Clear decimal mode flag 	        D
            CLD();
            frame_cycles += 2;
			break;
        case CLI_IMPL: 	    // Clear interrupt disable flag 	I
            CLI();
            frame_cycles += 2;
			break;
        case CLV_IMPL: 	    // Clear overflow flag 	            V
            CLV();
            frame_cycles += 2;
			break;
        case SEC_IMPL: 	    // Set carry flag 	                C
            SEC();
            frame_cycles += 2;
			break;
        case SED_IMPL: 	    // Set decimal mode flag 	        D
            SED();
            frame_cycles += 2;
			break;
        case SEI_IMPL: 	    // Set interrupt disable flag 	    I
            SEI();
            frame_cycles += 2;
			break;

        /* System Functions */
        case BRK_IMPL: 	    // Force an interrupt 	    B
            BRK();
            frame_cycles += 7;
            break;
        case NOP_IMPL: 	    // No Operation
            NOP();
            frame_cycles += 1;
            break;
        case RTI_IMPL:	    // Return from Interrupt 	All
            RTI();
            frame_cycles += 6;
            break;

        /* Unofficial/Illegal opcodes */
        /* Illegal NOP's */
        case NOP_IMM_ILL0:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMM_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMM_ILL2:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMM_ILL3:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMM_ILL4:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPG_ILL0:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPG_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPG_ILL2:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPGX_ILL0:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPGX_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPGX_ILL2:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPGX_ILL3:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPGX_ILL4:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ZPGX_ILL5:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMPL_ILL0:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMPL_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMPL_ILL2:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMPL_ILL3:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMPL_ILL4:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_IMPL_ILL5:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABS_ILL :
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABSX_ILL0:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABSX_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABSX_ILL2:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABSX_ILL3:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABSX_ILL4:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case NOP_ABSX_ILL5:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* JAM instructions cause the CPU to loop/halt indefinitely */
        case JAM_IMPL_ILL0:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL2:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL3:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL4:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL5:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL6:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL7:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL8:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL9:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL10:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case JAM_IMPL_ILL11:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        // TODO: compile remaining illegal opcodes 
        /* SLO = ASL combined with ORA */
        case SLO_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SLO_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SLO_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SLO_ZPGX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SLO_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SLO_ABS_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SLO_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* RLA = AND combined with ROL */
		case RLA_INDX_ILL:
        	VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
		case RLA_INDY_ILL:
        	VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
		case RLA_ZPG_ILL:
        	VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
		case RLA_ZPGX_ILL:
        	VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
		case RLA_ABSY_ILL:
        	VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
		case RLA_ABS_ILL:
        	VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
		case RLA_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* SRE = LSR combined with EOR */
        case SRE_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SRE_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SRE_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SRE_ZPGX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SRE_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SRE_ABS_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SRE_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* RRA = ROR combined with ADC */
        case RRA_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case RRA_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case RRA_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case RRA_ZPGX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case RRA_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case RRA_ABS_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case RRA_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* SAX */
        case SAX_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SAX_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SAX_ZPGY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SAX_ABS_ILL:

            /* LAX = LDA combined with LDX */
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAX_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAX_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAX_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAX_ZPGY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAX_ABS_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAX_ABSY_ILL:

            /* DCP = LDA combined with TSX */
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_ZPGX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_ABS_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case DCP_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* ISC = INC combined with SBC */
        case ISC_INDX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ISC_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ISC_ZPG_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ISC_ZPGX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ISC_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ISC_ABS_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ISC_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        /* ANC = AND combined with set C */
        case ANC_IMM_ILL0:
            //VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
            VNES_LOG::LOG(VNES_LOG::WARN, "Executing illegal opcode that happens to have implementation.");
            ANC_ILL();
            frame_cycles += 2;
			break;
        case ANC_IMM_ILL1:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;

        // misc
        case ALR_IMM_ILL:    // ALR = AND combined with LSR
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ARR_IMM_ILL:    // ARR = AND combined with ROR
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case ANE_IMM_ILL:    // ANE = ANDX combined with AND
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SHA_INDY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SHA_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SHY_ABSX_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SHX_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case TAS_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LXA_IMM_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case LAS_ABSY_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case SBX_IMM_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
			break;
        case USBC_IMM_ILL:
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
            break;
        default:
            VNES_LOG::LOG(VNES_LOG::FATAL, "Got unreachable instruction! How is this possible??? Bad instruction was %s\n", instruction);
            break;
    }
    //VNES_LOG::LOG(VNES_LOG::ERROR, "Cycle counting not implemented yet! Returning 1 cycle passed");
    return 0;
} // execute_instruction


void CPU::LDA(enum ADDRESSING_MODE mode){
    accumulator = read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
}

void CPU::LDX(enum ADDRESSING_MODE mode){
    index_X = read_mem(fetch_address(mode, AddrModeVec{ABSY}));
    zero_f      = (index_X == 0);
    negative_f  = (index_X & 0b10000000);
}

void CPU::LDY(enum ADDRESSING_MODE mode){
    index_Y = read_mem(fetch_address(mode, AddrModeVec{ABSX}));
    zero_f      = (index_Y == 0);
    negative_f  = (index_Y & 0b10000000);
}

void CPU::STA(enum ADDRESSING_MODE mode){
    write_mem(fetch_address(mode), accumulator);
}

void CPU::STX(enum ADDRESSING_MODE mode){
    write_mem(fetch_address(mode), index_X);
}

void CPU::STY(enum ADDRESSING_MODE mode){
    write_mem(fetch_address(mode), index_Y);
}


/* Register Transfers */
void CPU::TAX(){
    index_X = accumulator;
    zero_f      = (index_X == 0);
    negative_f  = (index_X & 0b10000000);
}

void CPU::TAY(){
    index_Y = accumulator;
    zero_f      = (index_Y == 0);
    negative_f  = (index_Y & 0b10000000);
}

void CPU::TXA(){
    accumulator = index_X;
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
}

void CPU::TYA(){
    accumulator = index_Y;
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
}


/* Stack Operations */
void CPU::TSX(){
    index_X = stack_pointer;
    zero_f      = (index_X == 0);
    negative_f  = (index_X & 0b10000000);
}

void CPU::TXS(){
    stack_pointer = index_X;
}

void CPU::PHA(){
    push_stack(accumulator);
}

void CPU::PHP(){
    push_stack(status_as_int());
}

void CPU::PLA(){
    VNES_LOG::LOG(VNES_LOG::Severity::WARN, "Am I implemented correctly? What does 'pull accumulator from stack' mean?");
    accumulator = pop_stack();
    zero_f = (accumulator == 0);
    negative_f = (accumulator & 0b10000000);
}

void CPU::PLP(){
    VNES_LOG::LOG(VNES_LOG::Severity::WARN, "Am I implemented correctly? What does 'pull processor status from stack' mean?");
    set_status_reg(pop_stack());
}


/* Logical */
void CPU::AND(enum ADDRESSING_MODE mode){
    accumulator = accumulator & read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
}

void CPU::EOR(enum ADDRESSING_MODE mode){
    accumulator = accumulator ^ read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
}

void CPU::ORA(enum ADDRESSING_MODE mode){
    accumulator = accumulator | read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
}

void CPU::BIT(enum ADDRESSING_MODE mode){
    uint8_t val = read_mem(fetch_address(mode));
    uint8_t result = accumulator & val;
    zero_f      = (result == 0);
    negative_f  = (val & 0b10000000);
    overflow_f  = (val & 0b01000000);
}


/* Arithmetic */
void CPU::ADC(enum ADDRESSING_MODE mode){
    // get the result as uint16_t so we can more easily check overflow
    uint16_t result = (uint16_t)accumulator + read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));
    result          += (uint16_t)carry_f;
    carry_f     = ((uint8_t)result & 0b100000000); // carry_f = result[8]
    overflow_f  = (accumulator & 0b10000000) != ((uint8_t)result & 0b10000000);
    negative_f  = ((uint8_t)result & 0b10000000);
    zero_f      = ((uint8_t)result == 0);
    accumulator = (uint8_t)result;
}

// this is kinda a mess but it works
void CPU::SBC(enum ADDRESSING_MODE mode){
    uint8_t data = read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));

    // sign 2's complement
    uint8_t sign_2_subtractor = ((data + carry_f) ^ 0xFF) + 1;
    uint16_t result = (uint16_t)accumulator + sign_2_subtractor;

    carry_f     = (( int8_t)result >= 0); // carry_f = result[8]
    negative_f  = ((uint8_t)result & 0b10000000);
    zero_f      = ((uint8_t)result == 0);
    int overflow_check = (int)(int8_t)accumulator - (int)(int8_t)data - carry_f; // weird casting gets signs and bit width correct
    overflow_f  = (overflow_check < -127 || overflow_check > 127);

    accumulator = (uint8_t)result;
}

void CPU::CMP(enum ADDRESSING_MODE mode){
    uint8_t data = read_mem(fetch_address(mode, AddrModeVec{ABSX, ABSY, INDY}));
    zero_f      = (accumulator == data);
    negative_f  = ((accumulator - data) & 0b10000000);
    carry_f     = (accumulator >= data);
}

void CPU::CPX(enum ADDRESSING_MODE mode){
    uint8_t data = read_mem(fetch_address(mode));
    zero_f      = (index_X == data);
    carry_f     = (index_X >= data);
    negative_f  = ((index_X - data) & 0b10000000);
}

void CPU::CPY(enum ADDRESSING_MODE mode){
    uint8_t data = read_mem(fetch_address(mode));
    zero_f      = (index_Y == data);
    negative_f  = ((index_Y - data) & 0b10000000);
    carry_f     = (index_Y >= data);
}


/* Increments & Decrements */
void CPU::INC(enum ADDRESSING_MODE mode){
    uint16_t addr = fetch_address(mode);
    uint8_t data = read_mem(addr);
    data++;
    negative_f  = (data & 0b10000000);
    zero_f      = (data == 0);
    write_mem(addr, data);
}

void CPU::INX(){
    index_X++;
    negative_f  = (index_X & 0b10000000);
    zero_f      = (index_X == 0);
}

void CPU::INY(){
    index_Y++;
    negative_f  = (index_Y & 0b10000000);
    zero_f      = (index_Y == 0);
}

void CPU::DEC(enum ADDRESSING_MODE mode){
    uint16_t addr = fetch_address(mode);
    int8_t data = read_mem(addr); // cast to signed
    data--;
    negative_f  = (data & 0b10000000);
    zero_f      = (data == 0);
    write_mem(addr, data);
}

void CPU::DEX(){
    index_X--;
    negative_f  = (index_X & 0b10000000);
    zero_f      = (index_X == 0);
}

void CPU::DEY(){
    index_Y--;
    negative_f  = (index_Y & 0b10000000);
    zero_f      = (index_Y == 0);
}


/* Shifts */
void CPU::ASL(enum ADDRESSING_MODE mode){
    uint16_t addr = fetch_address(mode);
    uint8_t data = read_mem(addr);;
    carry_f     = (data & 0b10000000); // the top bit is shifted into carry_f
    data = data << 1;
    negative_f  = (data & 0b10000000);
    zero_f      = (data == 0);
    write_mem(addr, data);
}

void CPU::ASL_eACC(){
    carry_f     = (accumulator & 0b10000000); // the top bit is shifted into carry_f
    accumulator = accumulator << 1;
    negative_f  = (accumulator & 0b10000000);
    zero_f      = (accumulator == 0);
}

void CPU::LSR(enum ADDRESSING_MODE mode){
    uint16_t addr = fetch_address(mode);
    uint8_t data = read_mem(addr);;
    carry_f     = (data & 0b00000001); // lowest bit shifts into carry_f
    negative_f  = 0; // result is always positive (bit 7 always 0)
    data = data >> 1; 
    zero_f      = (data == 0);
    write_mem(addr, data);
}

void CPU::LSR_eACC(){
    carry_f     = (accumulator & 0b00000001); // lowest bit shifts into carry_f
    negative_f  = 0; // result is always positive (bit 7 always 0)
    accumulator = accumulator >> 1; 
    zero_f      = (accumulator == 0);
}

void CPU::ROL(enum ADDRESSING_MODE mode){
    uint16_t addr = fetch_address(mode);
    uint8_t data = read_mem(addr);

    bit new_bit0    = carry_f;
    carry_f         = (data & 0b10000000);
    data            = data << 1;
    data            |= new_bit0;

    negative_f  = (data & 0b10000000);
    zero_f      = (data == 0);
    write_mem(addr, data);
}

void CPU::ROL_eACC(){
    bit new_bit0    = carry_f;
    carry_f         = (accumulator & 0b10000000);
    accumulator     = accumulator << 1;
    accumulator     |= new_bit0;

    negative_f  = (accumulator & 0b10000000);
    zero_f      = (accumulator == 0);
}

void CPU::ROR(enum ADDRESSING_MODE mode){
    uint16_t addr = fetch_address(mode);
    uint8_t data = read_mem(addr);

    bit new_carry_f = (data & 0b00000001);
    data = data >> 1;

    if(carry_f) data |= 0b10000000;
    negative_f  = carry_f;
    zero_f      = (data == 0);
    carry_f     = new_carry_f;

    write_mem(addr, data);
}

void CPU::ROR_eACC(){
    bit new_carry_f = (accumulator & 0b00000001);
    accumulator = accumulator >> 1;

    if(carry_f) accumulator |= 0b10000000;
    negative_f  = carry_f;
    zero_f      = (accumulator == 0);
    carry_f     = new_carry_f;
}


/* Jumps & Calls */
void CPU::JMP(enum ADDRESSING_MODE mode){
    program_counter = fetch_address(mode);
    program_counter--; // must bring PC back 1 byte to move from 1 to 0-indexing
}

void CPU::JSR(){
    push_stack((program_counter + 2) >> 8);
    push_stack(program_counter + 2);

    uint8_t PCL = read_mem(++program_counter);
    uint8_t PCH = read_mem(++program_counter);
    program_counter = ((uint16_t)PCH << 8 | PCL);
    program_counter--; // must bring PC back 1 byte to move from 1 to 0-indexing
}

void CPU::RTS(){
    uint8_t PCL = pop_stack();
    uint8_t PCH = pop_stack();
    program_counter = ((uint16_t)PCH << 8 | PCL);
}


/* Branches */
void CPU::branch(bit condition){
    if(condition){
        VNES_LOG::LOG(VNES_LOG::DEBUG, "branch(): took branch");
        int8_t rel_addr = fetch_address(REL);
        frame_cycles++; // add cycle if branch taken
        add_cycle_if_page_crossed(program_counter - 1, rel_addr, REL, AddrModeVec{REL}); // add cycle if branch crosses page
        program_counter += rel_addr;
    }else{
        VNES_LOG::LOG(VNES_LOG::DEBUG, "branch(): did NOT take branch");
        program_counter++;
    }
}

void CPU::BCC(){
    branch(!carry_f);
}

void CPU::BCS(){
    branch(carry_f);
}

void CPU::BEQ(){
    branch(zero_f);
}

void CPU::BMI(){
    branch(negative_f);
}

void CPU::BNE(){
    branch(!zero_f);
}

void CPU::BPL(){
    branch(!negative_f);
}

void CPU::BVC(){
    branch(!overflow_f);
}

void CPU::BVS(){
    branch(overflow_f);
}


/* Status Flag Changes */
void CPU::CLC(){
    carry_f = 0;
}

void CPU::CLD(){
    decimal_f = 0;
}

void CPU::CLI(){
    interrupt_disable_f = 0;
}

void CPU::CLV(){
    overflow_f = 0;
}

void CPU::SEC(){
    carry_f = 1;
}

void CPU::SED(){
    decimal_f = 1;
}

void CPU::SEI(){
    interrupt_disable_f = 1;
}


/* System Functions */
void CPU::BRK(){
    VNES_LOG::LOG(VNES_LOG::WARN, "Should BRK() handled the byte after the BRK opcode (byte is 'reason for interrupt/brk')? Investigate this!");
    program_counter += 2;
    raise_interrupt(true); // raise as maskable
}

void CPU::NOP(){
    // what, you looking for something here?
}

void CPU::RTI(){
    set_status_reg(pop_stack());
    uint8_t PCL = (pop_stack());
    uint8_t PCH = pop_stack();
    program_counter = ((uint16_t)PCH << 8) | PCL;
}


/* Unofficial/Illegal opcodes */
void CPU::NOP_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::JAM_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SLO_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::RLA_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SRE_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::RRA_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SAX_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::LAX_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::DCP_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::ISC_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::ANC_ILL(){
    accumulator = accumulator & read_mem(fetch_address(IMM, AddrModeVec{ABSX, ABSY, INDY}));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000);
    carry_f     = (accumulator & 0b10000000);
}

void CPU::ALR_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::ARR_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::ANE_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SHA_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SHY_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SHX_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::TAS_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::LXA_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::LAS_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::SBX_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

void CPU::USBC_ILL(){
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
}

