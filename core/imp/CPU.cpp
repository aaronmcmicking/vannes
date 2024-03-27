#pragma once

#include <iostream>
#include "../CPU.hpp"
#include "../../common/nes_assert.hpp"
#include "../../common/log.hpp"

CPU::CPU(RAM& _ram): ram {_ram}{
    VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::INFO, "Constructing CPU...");
}

inline uint8_t CPU::fetch_instruction(){
    return read_mem(program_counter);
}

inline void CPU::set_addr_mode(enum CPU_ADDRESSING_MODE mode){
    addr_mode = mode;
}

inline uint8_t CPU::read_mem(uint16_t addr){
    return ram.read(addr);
}

inline void CPU::write_mem(uint16_t addr, uint8_t data){
    ram.write(addr, data);
}

void CPU::push_stack(uint8_t data){
    ram.write(stack_pointer--, data);
}

uint8_t CPU::pop_stack(){
    return ram.read(stack_pointer++);
}


void CPU::write_nmi_vec(uint16_t data){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented correctly!");
    ram.write(RAM::VEC_ADDR::NMI_VEC, (uint8_t)data); // LB
    ram.write(RAM::VEC_ADDR::NMI_VEC + 1, (uint8_t)(data >> 8)); // HB
}

uint16_t CPU::read_nmi_vec(){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented correctly!");
    uint16_t data = 0;
    data |= read_mem(RAM::VEC_ADDR::NMI_VEC + 1); // HB
    data <<= 8;
    data |= read_mem(RAM::VEC_ADDR::NMI_VEC); // LB
    return data;
}

void CPU::write_reset_vec(uint16_t data){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented correctly!");
    ram.write(RAM::VEC_ADDR::RESET_VEC, (uint8_t)data); // LB
    ram.write(RAM::VEC_ADDR::RESET_VEC + 1, (uint8_t)(data >> 8)); // HB
}

uint16_t CPU::read_reset_vec(){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented correctly!");
    uint16_t data = 0;
    data |= read_mem(RAM::VEC_ADDR::RESET_VEC + 1); // HB
    data <<= 8;
    data |= read_mem(RAM::VEC_ADDR::RESET_VEC); // LB
    return data;
}

void CPU::write_brk_vec(uint16_t data){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented correctly!");
    ram.write(RAM::VEC_ADDR::BRK_VEC, (uint8_t)data); // LB
    ram.write(RAM::VEC_ADDR::BRK_VEC + 1, (uint8_t)(data >> 8)); // HB
}

uint16_t CPU::read_brk_vec(){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented correctly!");
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
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::INFO, "Entered Power-Up sequence");
    VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented right!");
    interrupt_disable = 1;
    program_counter = read_reset_vec();
    VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::INFO, "Loaded RESET Vec to PC. RESET Vec was %d. Can now enter normal execution cycle", read_reset_vec());
}

void CPU::engage_reset(){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::INFO, "Engaged reset signal");
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::ERROR, "Reset signal is not implemented!");
}

void CPU::release_reset(){
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::INFO, "Releasing reset signal");
	VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::ERROR, "Reset signal is not implemented!");
}

// Interrupt routines are different for maskable and non-maskable
// interrupts. However, they follow the same structure and the
// return-from-interrupt sequence is always the same (see: RTI)
void CPU::raise_interrupt(bool maskable){
    if(maskable && interrupt_disable){
        return;
    }
    push_stack((uint8_t)(program_counter >> 8)); // store PCH
    push_stack((uint8_t)program_counter); // store PCL
    push_stack(status_as_int()); // since I must have been low for the interrupt
                                 // and P is stored before I is asserted, 
                                 // I will always be desasserted by RTI
    interrupt_disable = 1; // the interrupt program may reassert this later
    uint16_t pc = 0;
    if(maskable){
        pc = read_brk_vec();
    }else{
        pc = read_nmi_vec();
    }
}

void CPU::return_from_interrupt(){
}

uint8_t CPU::status_as_int(){ 
    VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::WARN, "Check that I'm implemented right!");
    uint8_t status = 0b00100000; // bit 5 is always 1
    if(negative)            status |= 0b10000000;
    if(overflow)            status |= 0b01000000;
    if(b_flag)              status |= 0b00010000;
    if(decimal)             status |= 0b00001000;
    if(interrupt_disable)   status |= 0b00000100;
    if(zero)                status |= 0b00000010;
    if(carry)               status |= 0b00000001;
    return status;
}

// TODO: verify that page wrapping / page crossing is implemented correctly
uint16_t CPU::fetch_address(){
    using namespace VNES_LOG;
    uint16_t addr = 0;
    uint8_t zpg_ptr = 0; // a pointer into the zero page is sometimes needed
    switch(addr_mode){
        case ACC:
            addr = 0; // The accumulator is not in RAM.
                      // Instructions which work on the accumulator should not attempt to
                      // access RAM
            log(__FILE__, __LINE__, WARN, "Tried to fetch address while in Accumulator mode. This attempt shouldn't happen!\n");
			break;
		case ABS:
            // ABS has low, then high byte of programmer's desired address at PC+1 and PC+2
            addr |= read_mem(++program_counter);
            addr |= ((uint16_t)read_mem(++program_counter) << 8);
			break;
		case ABSX:
            addr |= read_mem(++program_counter);
            addr |= ((uint16_t)read_mem(++program_counter) << 8);
            addr += index_X;
			break;
		case ABSY:
            addr |= read_mem(++program_counter);
            addr |= ((uint16_t)read_mem(++program_counter) << 8);
            addr += index_Y;
			break;
		case IMM:
            addr = ++program_counter;
			break;
		case IMPL:
            addr = 0; // Implicit addressing never needs to fetch an address
            log(__FILE__, __LINE__, INFO, "Tried to fetch address while in Implicit mode. This is usually alright!\n");
			break;
		case IND:
            zpg_ptr = read_mem(++program_counter);
            addr |= read_mem(zpg_ptr);
            addr |= (read_mem(++zpg_ptr) << 8);
			break;
		case XIND:
            zpg_ptr = read_mem(++program_counter);
            zpg_ptr += index_X;
            addr |= read_mem(zpg_ptr);
            addr |= (read_mem(++zpg_ptr) << 8);
			break;
		case INDY:
            zpg_ptr = read_mem(++program_counter);
            zpg_ptr += index_Y;
            addr |= read_mem(zpg_ptr);
            addr |= (read_mem(++zpg_ptr) << 8);
			break;
		case REL:
            addr = ++program_counter;
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
            log(__FILE__, __LINE__, FATAL, "Tried to fetch address with bad addressing mode! Bad mode was %c", addr_mode);
            VNES_ASSERT(0 && "Unreachable");
            break;
    }
    return addr;
}

void CPU::execute_instruction(uint8_t instruction){
    switch(instruction){
        /* Load/Store */
        case LDA_XIND:    // Load Accumulator 	N,Z
            addr_mode = XIND;
            LDA();
            break;
        case LDA_ZPG:    
            addr_mode = ZPG;
            LDA();
            break;
        case LDA_IMM:    
            addr_mode = IMM;
            LDA();
            break;
        case LDA_ABS:    
            addr_mode = ABS;
            LDA();
            break;
        case LDA_INDY:    
            addr_mode = INDY;
            LDA();
            break;
        case LDA_ZPGX:    
            addr_mode = ZPGX;
            LDA();
            break;
        case LDA_ABSY:    
            addr_mode = ABSY;
            LDA();
            break;
        case LDA_ABSX:    
            addr_mode = ABSX;
            LDA();
            break;
        case LDX_IMM: 	// Load X Register 	    N,Z
            addr_mode = IMM;
            LDX();
            break;
        case LDX_ZPG: 	
            addr_mode = ZPG;
            LDX();
            break;
        case LDX_ABS: 	
            addr_mode = ABS;
            LDX();
            break;
        case LDX_ZPGY: 	
            addr_mode = ZPGY;
            LDX();
            break;
        case LDX_ABSY: 	
            addr_mode = ABSY;
            LDX();
            break;
        case LDY_IMM: 	// Load Y Register 	    N,Z
            addr_mode = IMM;
            LDY();
            break;
        case LDY_ZPG: 	
            addr_mode = ZPG;
            LDY();
            break;
        case LDY_ABS: 	
            addr_mode = ABS;
            LDY();
            break;
        case LDY_ZPGX: 	
            addr_mode = ZPGX;
            LDY();
            break;
        case LDY_ABSX: 	
            addr_mode = ABSX;
            LDY();
            break;
        case STA_XIND: 	// Store Accumulator 	 
            addr_mode = XIND;
            STA();
            break;
        case STA_ZPG: 	 	 
            addr_mode = ZPG;
            STA();
            break;
        case STA_ABS: 	 	 
            addr_mode = ABS;
            STA();
            break;
        case STA_INDY: 	 	 
            addr_mode = INDY;
            STA();
            break;
        case STA_ZPGX: 	 	 
            addr_mode = ZPGX;
            STA();
            break;
        case STA_ABSY: 	 	 
            addr_mode = ABSY;
            STA();
            break;
        case STA_ABSX: 	 	 
            addr_mode = ABSX;
            STA();
            break;
        case STX_ZPG: 	// Store X Register 	 
            addr_mode = ZPG;
            STX();
            break;
        case STX_ABS: 	 	 
            addr_mode = ABS;
            STX();
            break;
        case STX_ZPGY: 	 	 
            addr_mode = ZPGY;
            STX();
            break;
        case STY_ZPG: 	// Store Y Register 	 
            addr_mode = ZPG;
            STY();
            break;
        case STY_ABS: 	 	 
            addr_mode = ABS;
            STY();
            break;
        case STY_ZPGX: 	 	 
            addr_mode = ZPGX;
            STY();
            break;

        /* Register Transfers */
        case TAX_IMPL: 	    // Transfer accumulator to X 	N,Z
            addr_mode = IMPL;
			TAX();
			break;
        case TAY_IMPL: 	    // Transfer accumulator to Y 	N,Z
            addr_mode = IMPL;
			TAY();
			break;
        case TXA_IMPL: 	    // Transfer X to accumulator 	N,Z
            addr_mode = IMPL;
			TXA();
			break;
        case TYA_IMPL: 	    // Transfer Y to accumulator 	N,Z
            addr_mode = IMPL;
            TYA();
            break;

        /* Stack Operations */
        case TSX_IMPL: 	    // Transfer stack pointer to X 	    N,Z
            addr_mode = IMPL;
			TSX();
			break;
        case TXS_IMPL: 	    // Transfer X to stack pointer 	 
            addr_mode = IMPL;
			TXS();
			break;
        case PHA_IMPL: 	    // Push accumulator on stack 	 
            addr_mode = IMPL;
			PHA();
			break;
        case PHP_IMPL: 	    // Push processor status on stack 	 
            addr_mode = IMPL;
			PHP();
			break;
        case PLA_IMPL: 	    // Pull accumulator from stack 	    N,Z
            addr_mode = IMPL;
			PLA();
			break;
        case PLP_IMPL: 	    // Pull processor status from stack All
            addr_mode = IMPL;
			PLP();
            break;

        /* Logical */
        case AND_XIND: 	    // Logical AND 	            N,Z
            addr_mode = XIND;
            AND();
            break;
        case AND_ZPG: 	    
            addr_mode = ZPG;
            AND();
            break;
        case AND_IMM: 	    
            addr_mode = IMM;
            AND();
            break;
        case AND_ABS: 	    
            addr_mode = ABS;
            AND();
            break;
        case AND_INDY: 	    
            addr_mode = INDY;
            AND();
            break;
        case AND_ZPGX: 	    
            addr_mode = ZPGX;
            AND();
            break;
        case AND_ABSY: 	    
            addr_mode = ABSY;
            AND();
            break;
        case AND_ABSX: 	    
            addr_mode = ABSX;
            AND();
            break;
        case EOR_XIND: 	    // Exclusive OR     N,Z
            addr_mode = XIND;
            EOR();
            break;
        case EOR_ZPG: 	    
            addr_mode = ZPG;
            EOR();
            break;
        case EOR_IMM: 	    
            addr_mode = IMM;
            EOR();
            break;
        case EOR_ABS: 	    
            addr_mode = ABS;
            EOR();
            break;
        case EOR_INDY: 	    
            addr_mode = INDY;
            EOR();
            break;
        case EOR_ZPGX: 	    
            addr_mode = ZPGX;
            EOR();
            break;
        case EOR_ABSY: 	    
            addr_mode = ABSY;
            EOR();
            break;
        case EOR_ABSX: 	    
            addr_mode = ABSX;
            EOR();
            break;
        case ORA_XIND: 	    // Logical Inclusive OR 	N,Z
            addr_mode = XIND;
            ORA();
            break;
        case ORA_ZPG: 	    
            addr_mode = ZPG;
            ORA();
            break;
        case ORA_IMM: 	    
            addr_mode = IMM;
            ORA();
            break;
        case ORA_ABS: 	    
            addr_mode = ABS;
            ORA();
            break;
        case ORA_INDY: 	    
            addr_mode = INDY;
            ORA();
            break;
        case ORA_ZPGX: 	    
            addr_mode = ZPGX;
            ORA();
            break;
        case ORA_ABSY: 	    
            addr_mode = ABSY;
            ORA();
            break;
        case ORA_ABSX: 	    
            addr_mode = ABSX;
            ORA();
            break;
        case BIT_ZPG: 	    // Bit Test 	            N,V,Z
            addr_mode = ZPG;
            BIT();
            break;
        case BIT_ABS: 	    
            addr_mode = ABS;
            BIT();
            break;

        /* Arithmetic */
        case ADC_XIND: 	    // Add with Carry 	    N,V,Z,C 
            addr_mode = XIND;
            ADC();
            break;
        case ADC_ZPG: 	    
            addr_mode = ZPG;
            ADC();
            break;
        case ADC_IMM: 	    
            addr_mode = IMM;
            ADC();
            break;
        case ADC_ABS: 	    
            addr_mode = ABS;
            ADC();
            break;
        case ADC_INDY: 	    
            addr_mode = INDY;
            ADC();
            break;
        case ADC_ZPGX: 	    
            addr_mode = ZPGX;
            ADC();
            break;
        case ADC_ABSY: 	    
            addr_mode = ABSY;
            ADC();
            break;
        case ADC_ABSX: 	    
            addr_mode = ABSX;
            ADC();
            break;
        case SBC_XIND: 	    // Subtract with Carry 	N,V,Z,C
            addr_mode = XIND;
            SBC();
            break;
        case SBC_ZPG: 	    
            addr_mode = ZPG;
            SBC();
            break;
        case SBC_IMM: 	    
            addr_mode = IMM;
            SBC();
            break;
        case SBC_ABS: 	    
            addr_mode = ABS;
            SBC();
            break;
        case SBC_INDY: 	    
            addr_mode = INDY;
            SBC();
            break;
        case SBC_ZPGX: 	    
            addr_mode = ZPGX;
            SBC();
            break;
        case SBC_ABSY: 	    
            addr_mode = ABSY;
            SBC();
            break;
        case SBC_ABSX: 	    
            addr_mode = ABSX;
            SBC();
            break;
        case CMP_XIND: 	    // Compare accumulator 	N,Z,C
            addr_mode = XIND;
            CMP();
            break;
        case CMP_ZPG: 	    
            addr_mode = ZPG;
            CMP();
            break;
        case CMP_IMM: 	    
            addr_mode = IMM;
            CMP();
            break;
        case CMP_ABS: 	    
            addr_mode = ABS;
            CMP();
            break;
        case CMP_INDY: 	    
            addr_mode = INDY;
            CMP();
            break;
        case CMP_ZPGX: 	    
            addr_mode = ZPGX;
            CMP();
            break;
        case CMP_ABSY: 	    
            addr_mode = ABSY;
            CMP();
            break;
        case CMP_ABSX: 	    
            addr_mode = ABSX;
            CMP();
            break;
        case CPX_IMM: 	    // Compare X register 	N,Z,C
            addr_mode = IMM;
            CPX();
            break;
        case CPX_ZPG: 	    
            addr_mode = ZPG;
            CPX();
            break;
        case CPX_ABS: 	    
            addr_mode = ABS;
            CPX();
            break;
        case CPY_IMM: 	    // Compare Y register 	N,Z,C
            addr_mode = IMM;
            CPY();
            break;
        case CPY_ZPG: 	    
            addr_mode = ZPG;
            CPY();
            break;
        case CPY_ABS: 	    
            addr_mode = ABS;
            CPY();
            break;

        /* Increments & Decrements */
        case INC_ZPG: 	    // Increment a memory location 	N,Z
            addr_mode = ZPG;
            INC();
            break;
        case INC_ABS: 	    
            addr_mode = ABS;
            INC();
            break;
        case INC_ZPGX: 	    
            addr_mode = ZPGX;
            INC();
            break;
        case INC_ABSX: 	    
            addr_mode = ABSX;
            INC();
            break;
        case INX_IMPL: 	    // Increment the X register 	N,Z
            addr_mode = IMPL;
            INX();
            break;
        case INY_IMPL: 	    // Increment the Y register 	N,Z
            addr_mode = IMPL;
            INY();
            break;
        case DEC_ZPG: 	    // Decrement a memory location 	N,Z
            addr_mode = ZPG;
            DEC();
            break;
        case DEC_ABS: 	    
            addr_mode = ABS;
            DEC();
            break;
        case DEC_ZPGX: 	    
            addr_mode = ZPGX;
            DEC();
            break;
        case DEC_ABSX: 	    
            addr_mode = ABSX;
            DEC();
            break;
        case DEX_IMPL: 	    // Decrement the X register 	N,Z
            addr_mode = IMPL;
            DEX();
            break;
        case DEY_IMPL: 	    // Decrement the Y register 	N,Z
            addr_mode = IMPL;
            DEY();
            break;

        /* Shifts */
        case ASL_ZPG: 	    // Arithmetic Shift Left 	N,Z,C
            addr_mode = ZPG;
            ASL();
            break;
        case ASL_ACC: 	    
            addr_mode = ACC;
            ASL_eACC();
            break;
        case ASL_ABS: 	    
            addr_mode = ABS;
            ASL();
            break;
        case ASL_ZPGX: 	    
            addr_mode = ZPGX;
            ASL();
            break;
        case ASL_ABSX: 	    
            addr_mode = ABSX;
            ASL();
            break;
        case LSR_ZPG: 	    // Logical Shift Right 	    N,Z,C
            addr_mode = ZPG;
            LSR();
            break;
        case LSR_ACC: 	    
            addr_mode = ACC;
            LSR_eACC();
            break;
        case LSR_ABS: 	    
            addr_mode = ABS;
            LSR();
            break;
        case LSR_ZPGX: 	    
            addr_mode = ZPGX;
            LSR();
            break;
        case LSR_ABSX: 	    
            addr_mode = ABSX;
            LSR();
            break;
        case ROL_ZPG: 	    // Rotate Left 	            N,Z,C
            addr_mode = ZPG;
            ROL();
            break;
        case ROL_ACC: 	    
            addr_mode = ACC;
            ROL_eACC();
            break;
        case ROL_ABS: 	    
            addr_mode = ABS;
            ROL();
            break;
        case ROL_ZPGX: 	    
            addr_mode = ZPGX;
            ROL();
            break;
        case ROL_ABSX: 	    
            addr_mode = ABSX;
            ROL();
            break;
        case ROR_ZPG: 	    // Rotate Right 	        N,Z,C
            addr_mode = ZPG;
            ROR();
            break;
        case ROR_ACC: 	    
            addr_mode = ACC;
            ROR_eACC();
            break;
        case ROR_ABS: 	    
            addr_mode = ABS;
            ROR();
            break;
        case ROR_ZPGX: 	    
            addr_mode = ZPGX;
            ROR();
            break;
        case ROR_ABSX: 	    
            addr_mode =  ABSX;
            ROR();
            break;

        /* Jumps & Calls */
        case JMP_ABS: 	    // Jump to another location 	 
            addr_mode = ABS;
            JMP();
            break;
        case JMP_IND: 	     	 
            addr_mode = IND;
            JMP();
            break;
        case JSR_ABS: 	    // Jump to a subroutine 	 
            addr_mode = ABS;
            JSR();
            break;
        case RTS_IMPL: 	    // Return from subroutine 	 
            addr_mode = IMPL;
            RTS();
            break;

        /* Branches */
        case BCC_REL: 	    // Branch if carry flag clear 	 
            addr_mode = REL;
			BCC();
			break;
        case BCS_REL: 	    // Branch if carry flag set 	 
            addr_mode = REL;
			BCS();
			break;
        case BEQ_REL: 	    // Branch if zero flag set 	 
            addr_mode = REL;
			BEQ();
			break;
        case BMI_REL: 	    // Branch if negative flag set 	 
            addr_mode = REL;
			BMI();
			break;
        case BNE_REL: 	    // Branch if zero flag clear 	 
            addr_mode = REL;
			BNE();
			break;
        case BPL_REL: 	    // Branch if negative flag clear 	 
            addr_mode = REL;
			BPL();
			break;
        case BVC_REL: 	    // Branch if overflow flag clear 	 
            addr_mode = REL;
			BVC();
			break;
        case BVS_REL: 	    // Branch if overflow flag set 	 
            addr_mode = REL;
			BVS();
			break;

        /* Status Flag Changes */
        case CLC_IMPL: 	    // Clear carry flag 	            C
            addr_mode = IMPL;
			CLC();
			break;
        case CLD_IMPL: 	    // Clear decimal mode flag 	        D
            addr_mode = IMPL;
			CLD();
			break;
        case CLI_IMPL: 	    // Clear interrupt disable flag 	I
            addr_mode = IMPL;
			CLI();
			break;
        case CLV_IMPL: 	    // Clear overflow flag 	            V
            addr_mode = IMPL;
			CLV();
			break;
        case SEC_IMPL: 	    // Set carry flag 	                C
            addr_mode = IMPL;
			SEC();
			break;
        case SED_IMPL: 	    // Set decimal mode flag 	        D
            addr_mode = IMPL;
			SED();
			break;
        case SEI_IMPL: 	    // Set interrupt disable flag 	    I
            addr_mode = IMPL;
			SEI();
			break;

        /* System Functions */
        case BRK_IMPL: 	    // Force an interrupt 	    B
            BRK();
            break;
        case NOP_IMPL: 	    // No Operation
            NOP();
            break;
        case RTI_IMPL:	    // Return from Interrupt 	All
            RTI();
            break;

        /* Unofficial/Illegal opcodes */
        /* Illegal NOP's */
        case NOP_IMM_ILL0:
            break;
        case NOP_IMM_ILL1:
            break;
        case NOP_IMM_ILL2:
            break;
        case NOP_IMM_ILL3:
            break;
        case NOP_IMM_ILL4:
            break;
        case NOP_ZPG_ILL0:
            break;
        case NOP_ZPG_ILL1:
            break;
        case NOP_ZPG_ILL2:
            break;
        case NOP_ZPGX_ILL0:
            break;
        case NOP_ZPGX_ILL1:
            break;
        case NOP_ZPGX_ILL2:
            break;
        case NOP_ZPGX_ILL3:
            break;
        case NOP_ZPGX_ILL4:
            break;
        case NOP_ZPGX_ILL5:
            break;
        case NOP_IMPL_ILL0:
            break;
        case NOP_IMPL_ILL1:
            break;
        case NOP_IMPL_ILL2:
            break;
        case NOP_IMPL_ILL3:
            break;
        case NOP_IMPL_ILL4:
            break;
        case NOP_IMPL_ILL5:
            break;
        case NOP_ABS_ILL :
            break;
        case NOP_ABSX_ILL0:
            break;
        case NOP_ABSX_ILL1:
            break;
        case NOP_ABSX_ILL2:
            break;
        case NOP_ABSX_ILL3:
            break;
        case NOP_ABSX_ILL4:
            break;
        case NOP_ABSX_ILL5:
            break;

        /* JAM instructions cause the CPU to loop/halt indefinitely */
        case JAM_IMPL_ILL0:
            break;
        case JAM_IMPL_ILL1:
            break;
        case JAM_IMPL_ILL2:
            break;
        case JAM_IMPL_ILL3:
            break;
        case JAM_IMPL_ILL4:
            break;
        case JAM_IMPL_ILL5:
            break;
        case JAM_IMPL_ILL6:
            break;
        case JAM_IMPL_ILL7:
            break;
        case JAM_IMPL_ILL8:
            break;
        case JAM_IMPL_ILL9:
            break;
        case JAM_IMPL_ILL10:
            break;
        case JAM_IMPL_ILL11:
            break;

        // TODO: compile remaining illegal opcodes 
        /* SLO = ASL combined with ORA */
        case SLO_XIND_ILL:
            break;
        case SLO_INDY_ILL:
            break;
        case SLO_ZPG_ILL:
            break;
        case SLO_ZPGX_ILL:
            break;
        case SLO_ABSY_ILL:
            break;
        case SLO_ABS_ILL:
            break;
        case SLO_ABSX_ILL:
            break;

        /* RLA = AND combined with ROL */
		case RLA_XIND_ILL:
        	break;
		case RLA_INDY_ILL:
        	break;
		case RLA_ZPG_ILL:
        	break;
		case RLA_ZPGX_ILL:
        	break;
		case RLA_ABSY_ILL:
        	break;
		case RLA_ABS_ILL:
        	break;
		case RLA_ABSX_ILL:
            break;

        /* SRE = LSR combined with EOR */
        case SRE_XIND_ILL:
            break;
        case SRE_INDY_ILL:
            break;
        case SRE_ZPG_ILL:
            break;
        case SRE_ZPGX_ILL:
            break;
        case SRE_ABSY_ILL:
            break;
        case SRE_ABS_ILL:
            break;
        case SRE_ABSX_ILL:
            break;

        /* RRA = ROR combined with ADC */
        case RRA_XIND_ILL:
            break;
        case RRA_INDY_ILL:
            break;
        case RRA_ZPG_ILL:
            break;
        case RRA_ZPGX_ILL:
            break;
        case RRA_ABSY_ILL:
            break;
        case RRA_ABS_ILL:
            break;
        case RRA_ABSX_ILL:
            break;

        /* SAX */
        case SAX_XIND_ILL:
            break;
        case SAX_ZPG_ILL:
            break;
        case SAX_ZPGY_ILL:
            break;
        case SAX_ABS_ILL:

            /* LAX = LDA combined with LDX */
            break;
        case LAX_XIND_ILL:
            break;
        case LAX_INDY_ILL:
            break;
        case LAX_ZPG_ILL:
            break;
        case LAX_ZPGY_ILL:
            break;
        case LAX_ABS_ILL:
            break;
        case LAX_ABSY_ILL:

            /* DCP = LDA combined with TSX */
            break;
        case DCP_XIND_ILL:
            break;
        case DCP_INDY_ILL:
            break;
        case DCP_ZPG_ILL:
            break;
        case DCP_ZPGX_ILL:
            break;
        case DCP_ABSY_ILL:
            break;
        case DCP_ABS_ILL:
            break;
        case DCP_ABSX_ILL:
            break;

        /* ISC = INC combined with SBC */
        case ISC_XIND_ILL:
            break;
        case ISC_INDY_ILL:
            break;
        case ISC_ZPG_ILL:
            break;
        case ISC_ZPGX_ILL:
            break;
        case ISC_ABSY_ILL:
            break;
        case ISC_ABS_ILL:
            break;
        case ISC_ABSX_ILL:
            break;

        /* ANC = AND combined with set C */
        case ANC_IMM_ILL0:
            break;
        case ANC_IMM_ILL1:
            break;

        // misc
        case ALR_IMM_ILL:    // ALR = AND combined with LSR
            break;
        case ARR_IMM_ILL:    // ARR = AND combined with ROR
            break;
        case ANE_IMM_ILL:    // ANE = ANDX combined with AND
            break;
        case SHA_INDY_ILL:
            break;
        case SHA_ABSY_ILL:
            break;
        case SHY_ABSX_ILL:
            break;
        case SHX_ABSY_ILL:
            break;
        case TAS_ABSY_ILL:
            break;
        case LXA_IMM_ILL:
            break;
        case LAS_ABSY_ILL:
            break;
        case SBX_IMM_ILL:
            break;
        case USBC_IMM_ILL:
            break;
        default:
            VNES_LOG::log(__FILE__, __LINE__, VNES_LOG::FATAL, "Got unreachable instruction! How is this possible??? Bad instruction was %s\n", instruction);
            break;
    }
} // execute_instruction


        void CPU::LDA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
     
        void CPU::LDX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void CPU::LDY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void CPU::STA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void CPU::STX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void CPU::STY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
     

        /* Register Transfers */
        void CPU::TAX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 
        void CPU::TAY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 
        void CPU::TXA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 
        void CPU::TYA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 

        /* Stack Operations */
        void CPU::TSX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::TXS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::PHA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::PHP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::PLA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::PLP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Logical */
        void CPU::AND(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::EOR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ORA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BIT(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Arithmetic */
        void CPU::ADC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SBC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::CMP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::CPX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::CPY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Increments & Decrements */
        void CPU::INC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::INX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::INY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::DEC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::DEX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::DEY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Shifts */
        void CPU::ASL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ASL_eACC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::LSR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::LSR_eACC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ROL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ROL_eACC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ROR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ROR_eACC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Jumps & Calls */
        void CPU::JMP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::JSR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::RTS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Branches */
        void CPU::BCC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BCS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BEQ(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BMI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BNE(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BPL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BVC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::BVS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Status Flag Changes */
        void CPU::CLC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::CLD(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::CLI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::CLV(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SEC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SED(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SEI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* System Functions */
        void CPU::BRK(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::NOP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::RTI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Unofficial/Illegal opcodes */
        void CPU::NOP_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::JAM_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SLO_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::RLA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SRE_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::RRA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SAX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::LAX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::DCP_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ISC_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ANC_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ALR_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ARR_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::ANE_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SHA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SHY_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SHX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::TAS_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::LXA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::LAS_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::SBX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPU::USBC_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

