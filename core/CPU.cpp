#pragma once

#include <iostream>
#include "include/CPU.hpp"
#include "../common/nes_assert.hpp"
#include "../common/log.hpp"

CPU::CPU(RAM& _ram, PPU& _ppu): ram {_ram}, ppu {_ppu}{
    VNES_LOG::LOG(VNES_LOG::INFO, "Constructing CPU...");
    power_up();
}

void CPU::step(){
    uint8_t opcode = fetch_instruction();
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Fetched opcode %x from address %x", opcode, program_counter);
    int cycles = execute_instruction(opcode);
    program_counter++;

    for(int i = 0; i < cycles*3; i++){ // 1 cpu cycle = 3 ppu cycles
        ppu.do_cycle();
    }
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
	VNES_LOG::LOG(VNES_LOG::WARN, "read_reset_vec(): Check that I'm implemented correctly!");
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
    VNES_LOG::LOG(VNES_LOG::WARN, "Check that I'm implemented right!");
    interrupt_disable_f = 1;
    //program_counter = read_reset_vec();
    reset();
    VNES_LOG::LOG(VNES_LOG::INFO, "Power-up sequence done");
}

void CPU::reset(){
    VNES_LOG::LOG(VNES_LOG::INFO, "Received reset signal");
    program_counter = read_reset_vec();
    VNES_LOG::LOG(VNES_LOG::INFO, "Loaded RESET Vec to PC. RESET Vec was %x.", read_reset_vec());
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

// TODO: verify that page wrapping / page crossing is implemented correctly
uint16_t CPU::fetch_address(enum ADDRESSING_MODE mode){
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
            LOG(ERROR, "Tried to fetch address while in Implied mode. This attempt shouldn't happen!\n");
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
    VNES_LOG::LOG(VNES_LOG::DEBUG, "Executing instruction %x", instruction);
    switch(instruction){
        /* Load/Store */
        case LDA_XIND:    // Load Accumulator 	N,Z
            LDA(XIND);
            break;
        case LDA_ZPG:    
            LDA(ZPG);
            break;
        case LDA_IMM:    
            LDA(IMM);
            break;
        case LDA_ABS:    
            LDA(ABS);
            break;
        case LDA_INDY:    
            LDA(INDY);
            break;
        case LDA_ZPGX:    
            LDA(ZPGX);
            break;
        case LDA_ABSY:    
            LDA(ABSY);
            break;
        case LDA_ABSX:    
            LDA(ABSX);
            break;
        case LDX_IMM: 	// Load X Register 	    N,Z
            LDX(IMM);
            break;
        case LDX_ZPG: 	
            LDX(ZPG);
            break;
        case LDX_ABS: 	
            LDX(ABS);
            break;
        case LDX_ZPGY: 	
            LDX(ZPGY);
            break;
        case LDX_ABSY: 	
            LDX(ABSY);
            break;
        case LDY_IMM: 	// Load Y Register 	    N,Z
            LDY(IMM);
            break;
        case LDY_ZPG: 	
            LDY(ZPG);
            break;
        case LDY_ABS: 	
            LDY(ABS);
            break;
        case LDY_ZPGX: 	
            LDY(ZPGX);
            break;
        case LDY_ABSX: 	
            LDY(ABSX);
            break;
        case STA_XIND: 	// Store Accumulator 	 
            STA(XIND);
            break;
        case STA_ZPG: 	 	 
            STA(ZPG);
            break;
        case STA_ABS: 	 	 
            STA(ABS);
            break;
        case STA_INDY: 	 	 
            STA(INDY);
            break;
        case STA_ZPGX: 	 	 
            STA(ZPGX);
            break;
        case STA_ABSY: 	 	 
            STA(ABSY);
            break;
        case STA_ABSX: 	 	 
            STA(ABSX);
            break;
        case STX_ZPG: 	// Store X Register 	 
            STX(ZPG);
            break;
        case STX_ABS: 	 	 
            STX(ABS);
            break;
        case STX_ZPGY: 	 	 
            STX(ZPGY);
            break;
        case STY_ZPG: 	// Store Y Register 	 
            STY(ZPG);
            break;
        case STY_ABS: 	 	 
            STY(ABS);
            break;
        case STY_ZPGX: 	 	 
            STY(ZPGX);
            break;

        /* Register Transfers */
        case TAX_IMPL: 	    // Transfer accumulator to X 	N,Z
            TAX();
			break;
        case TAY_IMPL: 	    // Transfer accumulator to Y 	N,Z
            TAY();
			break;
        case TXA_IMPL: 	    // Transfer X to accumulator 	N,Z
            TXA();
			break;
        case TYA_IMPL: 	    // Transfer Y to accumulator 	N,Z
            TYA();
            break;

        /* Stack Operations */
        case TSX_IMPL: 	    // Transfer stack pointer to X 	    N,Z
            TSX();
			break;
        case TXS_IMPL: 	    // Transfer X to stack pointer 	 
            TXS();
			break;
        case PHA_IMPL: 	    // Push accumulator on stack 	 
            PHA();
			break;
        case PHP_IMPL: 	    // Push processor status on stack 	 
            PHP();
			break;
        case PLA_IMPL: 	    // Pull accumulator from stack 	    N,Z
            PLA();
			break;
        case PLP_IMPL: 	    // Pull processor status from stack All
            PLP();
            break;

        /* Logical */
        case AND_XIND: 	    // Logical AND 	            N,Z
            AND(XIND);
            break;
        case AND_ZPG: 	    
            AND(ZPG);
            break;
        case AND_IMM: 	    
            AND(IMM);
            break;
        case AND_ABS: 	    
            AND(ABS);
            break;
        case AND_INDY: 	    
            AND(INDY);
            break;
        case AND_ZPGX: 	    
            AND(ZPGX);
            break;
        case AND_ABSY: 	    
            AND(ABSY);
            break;
        case AND_ABSX: 	    
            AND(ABSX);
            break;
        case EOR_XIND: 	    // Exclusive OR     N,Z
            EOR(XIND);
            break;
        case EOR_ZPG: 	    
            EOR(ZPG);
            break;
        case EOR_IMM: 	    
            EOR(IMM);
            break;
        case EOR_ABS: 	    
            EOR(ABS);
            break;
        case EOR_INDY: 	    
            EOR(INDY);
            break;
        case EOR_ZPGX: 	    
            EOR(ZPGX);
            break;
        case EOR_ABSY: 	    
            EOR(ABSY);
            break;
        case EOR_ABSX: 	    
            EOR(ABSX);
            break;
        case ORA_XIND: 	    // Logical Inclusive OR 	N,Z
            ORA(XIND);
            break;
        case ORA_ZPG: 	    
            ORA(ZPG);
            break;
        case ORA_IMM: 	    
            ORA(IMM);
            break;
        case ORA_ABS: 	    
            ORA(ABS);
            break;
        case ORA_INDY: 	    
            ORA(INDY);
            break;
        case ORA_ZPGX: 	    
            ORA(ZPGX);
            break;
        case ORA_ABSY: 	    
            ORA(ABSY);
            break;
        case ORA_ABSX: 	    
            ORA(ABSX);
            break;
        case BIT_ZPG: 	    // Bit Test 	            N,V,Z
            BIT(ZPG);
            break;
        case BIT_ABS: 	    
            BIT(ABS);
            break;

        /* Arithmetic */
        case ADC_XIND: 	    // Add with Carry 	    N,V,Z,C 
            ADC(XIND);
            break;
        case ADC_ZPG: 	    
            ADC(ZPG);
            break;
        case ADC_IMM: 	    
            ADC(IMM);
            break;
        case ADC_ABS: 	    
            ADC(ABS);
            break;
        case ADC_INDY: 	    
            ADC(INDY);
            break;
        case ADC_ZPGX: 	    
            ADC(ZPGX);
            break;
        case ADC_ABSY: 	    
            ADC(ABSY);
            break;
        case ADC_ABSX: 	    
            ADC(ABSX);
            break;
        case SBC_XIND: 	    // Subtract with Carry 	N,V,Z,C
            SBC(XIND);
            break;
        case SBC_ZPG: 	    
            SBC(ZPG);
            break;
        case SBC_IMM: 	    
            SBC(IMM);
            break;
        case SBC_ABS: 	    
            SBC(ABS);
            break;
        case SBC_INDY: 	    
            SBC(INDY);
            break;
        case SBC_ZPGX: 	    
            SBC(ZPGX);
            break;
        case SBC_ABSY: 	    
            SBC(ABSY);
            break;
        case SBC_ABSX: 	    
            SBC(ABSX);
            break;
        case CMP_XIND: 	    // Compare accumulator 	N,Z,C
            CMP(XIND);
            break;
        case CMP_ZPG: 	    
            CMP(ZPG);
            break;
        case CMP_IMM: 	    
            CMP(IMM);
            break;
        case CMP_ABS: 	    
            CMP(ABS);
            break;
        case CMP_INDY: 	    
            CMP(INDY);
            break;
        case CMP_ZPGX: 	    
            CMP(ZPGX);
            break;
        case CMP_ABSY: 	    
            CMP(ABSY);
            break;
        case CMP_ABSX: 	    
            CMP(ABSX);
            break;
        case CPX_IMM: 	    // Compare X register 	N,Z,C
            CPX(IMM);
            break;
        case CPX_ZPG: 	    
            CPX(ZPG);
            break;
        case CPX_ABS: 	    
            CPX(ABS);
            break;
        case CPY_IMM: 	    // Compare Y register 	N,Z,C
            CPY(IMM);
            break;
        case CPY_ZPG: 	    
            CPY(ZPG);
            break;
        case CPY_ABS: 	    
            CPY(ABS);
            break;

        /* Increments & Decrements */
        case INC_ZPG: 	    // Increment a memory location 	N,Z
            INC(ZPG);
            break;
        case INC_ABS: 	    
            INC(ABS);
            break;
        case INC_ZPGX: 	    
            INC(ZPGX);
            break;
        case INC_ABSX: 	    
            INC(ABSX);
            break;
        case INX_IMPL: 	    // Increment the X register 	N,Z
            INX();
            break;
        case INY_IMPL: 	    // Increment the Y register 	N,Z
            INY();
            break;
        case DEC_ZPG: 	    // Decrement a memory location 	N,Z
            DEC(ZPG);
            break;
        case DEC_ABS: 	    
            DEC(ABS);
            break;
        case DEC_ZPGX: 	    
            DEC(ZPGX);
            break;
        case DEC_ABSX: 	    
            DEC(ABSX);
            break;
        case DEX_IMPL: 	    // Decrement the X register 	N,Z
            DEX();
            break;
        case DEY_IMPL: 	    // Decrement the Y register 	N,Z
            DEY();
            break;

        /* Shifts */
        case ASL_ZPG: 	    // Arithmetic Shift Left 	N,Z,C
            ASL(ZPG);
            break;
        case ASL_ACC: 	    
            ASL_eACC();
            break;
        case ASL_ABS: 	    
            ASL(ABS);
            break;
        case ASL_ZPGX: 	    
            ASL(ZPGX);
            break;
        case ASL_ABSX: 	    
            ASL(ABSX);
            break;
        case LSR_ZPG: 	    // Logical Shift Right 	    N,Z,C
            LSR(ZPG);
            break;
        case LSR_ACC: 	    
            LSR_eACC();
            break;
        case LSR_ABS: 	    
            LSR(ABS);
            break;
        case LSR_ZPGX: 	    
            LSR(ZPGX);
            break;
        case LSR_ABSX: 	    
            LSR(ABSX);
            break;
        case ROL_ZPG: 	    // Rotate Left 	            N,Z,C
            ROL(ZPG);
            break;
        case ROL_ACC: 	    
            ROL_eACC();
            break;
        case ROL_ABS: 	    
            ROL(ABS);
            break;
        case ROL_ZPGX: 	    
            ROL(ZPGX);
            break;
        case ROL_ABSX: 	    
            ROL(ABSX);
            break;
        case ROR_ZPG: 	    // Rotate Right 	        N,Z,C
            ROR(ZPG);
            break;
        case ROR_ACC: 	    
            ROR_eACC();
            break;
        case ROR_ABS: 	    
            ROR(ABS);
            break;
        case ROR_ZPGX: 	    
            ROR(ZPGX);
            break;
        case ROR_ABSX: 	    
            ROR(ABS);
            break;

        /* Jumps & Calls */
        case JMP_ABS: 	    // Jump to another location 	 
            JMP(ABS);
            break;
        case JMP_IND: 	     	 
            JMP(IND);
            break;
        case JSR_ABS: 	    // Jump to a subroutine 	 
            JSR();
            break;
        case RTS_IMPL: 	    // Return from subroutine 	 
            RTS();
            break;

        /* Branches */
        case BCC_REL: 	    // Branch if carry flag clear 	 
            BCC();
			break;
        case BCS_REL: 	    // Branch if carry flag set 	 
            BCS();
			break;
        case BEQ_REL: 	    // Branch if zero flag set 	 
            BEQ();
			break;
        case BMI_REL: 	    // Branch if negative flag set 	 
            BMI();
			break;
        case BNE_REL: 	    // Branch if zero flag clear 	 
            BNE();
			break;
        case BPL_REL: 	    // Branch if negative flag clear 	 
            BPL();
			break;
        case BVC_REL: 	    // Branch if overflow flag clear 	 
            BVC();
			break;
        case BVS_REL: 	    // Branch if overflow flag set 	 
            BVS();
			break;

        /* Status Flag Changes */
        case CLC_IMPL: 	    // Clear carry flag 	            C
            CLC();
			break;
        case CLD_IMPL: 	    // Clear decimal mode flag 	        D
            CLD();
			break;
        case CLI_IMPL: 	    // Clear interrupt disable flag 	I
            CLI();
			break;
        case CLV_IMPL: 	    // Clear overflow flag 	            V
            CLV();
			break;
        case SEC_IMPL: 	    // Set carry flag 	                C
            SEC();
			break;
        case SED_IMPL: 	    // Set decimal mode flag 	        D
            SED();
			break;
        case SEI_IMPL: 	    // Set interrupt disable flag 	    I
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
        case SLO_XIND_ILL:
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
		case RLA_XIND_ILL:
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
        case SRE_XIND_ILL:
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
        case RRA_XIND_ILL:
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
        case SAX_XIND_ILL:
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
        case LAX_XIND_ILL:
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
        case DCP_XIND_ILL:
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
        case ISC_XIND_ILL:
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
            VNES_LOG::LOG(VNES_LOG::WARN, "Attempted to execute unimplemented illegal opcode. Continuing with no effect.");
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
    VNES_LOG::LOG(VNES_LOG::ERROR, "Cycle counting not implemented yet! Returning 1 cycle passed");
    return 1;
} // execute_instruction


void CPU::LDA(enum ADDRESSING_MODE mode){
    accumulator = read_mem(fetch_address(mode));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000) ? 1 : 0;
}

void CPU::LDX(enum ADDRESSING_MODE mode){
    index_X = read_mem(fetch_address(mode));
    zero_f      = (index_X == 0) ? 1 : 0;
    negative_f  = (index_X & 0b10000000) ? 1 : 0;
}

void CPU::LDY(enum ADDRESSING_MODE mode){
    index_Y = read_mem(fetch_address(mode));
    zero_f      = (index_Y == 0) ? 1 : 0;
    negative_f  = (index_Y & 0b10000000) ? 1 : 0;
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
    zero_f      = (index_X == 0) ? 1 : 0;
    negative_f  = (index_X & 0b10000000) ? 1 : 0;
}

void CPU::TAY(){
    index_Y = accumulator;
    zero_f      = (index_Y == 0) ? 1 : 0;
    negative_f  = (index_Y & 0b10000000) ? 1 : 0;
}

void CPU::TXA(){
    accumulator = index_X;
    zero_f      = (accumulator == 0) ? 1 : 0;
    negative_f  = (accumulator & 0b10000000) ? 1 : 0;
}

void CPU::TYA(){
    accumulator = index_Y;
    zero_f      = (accumulator == 0) ? 1 : 0;
    negative_f  = (accumulator & 0b10000000) ? 1 : 0;
}


/* Stack Operations */
void CPU::TSX(){
    index_X = stack_pointer;
    zero_f      = (index_X == 0) ? 1 : 0;
    negative_f  = (index_X & 0b10000000) ? 1 : 0;
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
    zero_f = (accumulator == 0) ? 1 : 0;
    negative_f = (accumulator & 0b10000000) ? 1 : 0;
}

void CPU::PLP(){
    VNES_LOG::LOG(VNES_LOG::Severity::WARN, "Am I implemented correctly? What does 'pull processor status from stack' mean?");
    set_status_reg(pop_stack());
}


/* Logical */
void CPU::AND(enum ADDRESSING_MODE mode){
    accumulator = accumulator & read_mem(fetch_address(mode));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000) ? 1 : 0;
}

void CPU::EOR(enum ADDRESSING_MODE mode){
    accumulator = accumulator ^ read_mem(fetch_address(mode));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000) ? 1 : 0;
}

void CPU::ORA(enum ADDRESSING_MODE mode){
    accumulator = accumulator | read_mem(fetch_address(mode));
    zero_f      = (accumulator == 0);
    negative_f  = (accumulator & 0b10000000) ? 1 : 0;
}

void CPU::BIT(enum ADDRESSING_MODE mode){
    uint8_t val = read_mem(fetch_address(mode));
    uint8_t result = accumulator & val;
    zero_f      = (result == 0);
    negative_f  = (val & 0b10000000) ? 1 : 0;
    overflow_f  = (val & 0b01000000) ? 1 : 0;
}


/* Arithmetic */
void CPU::ADC(enum ADDRESSING_MODE mode){
    // get the result as uint16_t so we can more easily check overflow
    uint16_t result = (uint16_t)accumulator + read_mem(fetch_address(mode)) + (uint16_t)carry_f;
    carry_f     = ((uint8_t)result & 0b100000000) ? 1 : 0; // carry_f = result[8]
    overflow_f  = (accumulator & 0b10000000) != ((uint8_t)result & 0b10000000);
    negative_f  = ((uint8_t)result & 0b10000000) ? 1 : 0;
    zero_f      = ((uint8_t)result == 0);
    accumulator = (uint8_t)result;
}

// this is kinda a mess but it works
void CPU::SBC(enum ADDRESSING_MODE mode){
    uint8_t data = read_mem(fetch_address(mode));

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
    uint8_t data = read_mem(fetch_address(mode));
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
}

void CPU::JSR(){
    push_stack((program_counter + 2) >> 8);
    push_stack(program_counter + 2);

    uint8_t PCL = read_mem(++program_counter);
    uint8_t PCH = read_mem(++program_counter);
    program_counter = ((uint16_t)PCH << 8 | PCL);
}

void CPU::RTS(){
    uint8_t PCL = pop_stack();
    uint8_t PCH = pop_stack();
    program_counter = ((uint16_t)PCH << 8 | PCL);
}


/* Branches */
void CPU::branch(bit condition){
    if(condition){
        int8_t rel_addr = fetch_address(REL);
        program_counter += rel_addr;
    }else{
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
    VNES_LOG::LOG(VNES_LOG::ERROR, "NOP is not implemented since cycles are unimplemented. NOP should consume a CPU cycle.");
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
	VNES_ASSERT(0 && "UNIMPLEMENTED ILLEGAL INSTRUCTION");
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

