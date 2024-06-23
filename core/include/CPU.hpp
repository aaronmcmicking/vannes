#pragma once

#include <stdint.h>
#include "../RAM.cpp"
#include "../PPU.cpp"
#include "../../common/typedefs.hpp"
#include <string>


// The NES CPU is a modified version of the MOS 6502 called the Ricoh 2A03.
// It removes some instructions from the 6502 and includes and APU in the CPU.

// TODO: clean up switching between public and private
class CPU{
    public:
        CPU(RAM& _ram, PPU& ppu_);
        void step();
        const bool MASKABLE_IRQ = false; // interrupts are not actually maskable, since implementing masking is hard and im dumb
        void reset();

    private:
        RAM& ram;
        PPU& ppu;

        /* registers */
        uint16_t program_counter;
        uint8_t stack_pointer;
        uint8_t accumulator;
        uint8_t index_X;
        uint8_t index_Y;

        /* status register */
        /* The in hardware status register is a single byte value with six
         * 1-bit flags. Bits 4 and 5 are officially unused: Bit 5 is always
         * driven to 1 and bit 4 is known as the "B flag" and is controlled by
         * CPU side effects (see https://www.nesdev.org/wiki/Status_flags#The_B_flag)
         */
        bit carry_f;
        bit zero_f;
        bit interrupt_disable_f;
        bit b_flag_f;
        bit decimal_f;
        bit overflow_f;
        bit negative_f;
    public: 
        uint8_t status_as_int(); // returns the packed status register
        void    set_status_reg(uint8_t data); 
    private:

        uint64_t frame_cycles;

        uint8_t fetch_instruction();
        int     execute_instruction(uint8_t instruction);

        // see https://www.masswerk.at/6502/6502_instruction_set.html#ADC
        enum ADDRESSING_MODE{
            ACC = 1,    // Accumulator 
            ABS,        // Absolute
            ABSX,       // Absolute, X-indexed
            ABSY,       // Absolute, Y-indexed
            IMM,        // Immediate
            IMPL,       // Implied
            IND,        // Indirect
            INDX,       // X-indexed, indirect
            INDY,       // Indirect, Y-indexed
            REL,        // Relative
            ZPG,        // Zeropage
            ZPGX,       // Zeropage, X-indexed
            ZPGY        // Zeropage, Y-indexed
        };

        typedef     std::vector<enum ADDRESSING_MODE> AddrModeVec;
        uint16_t    fetch_address(enum ADDRESSING_MODE mode, const AddrModeVec& page_crossing_modes);
        void        add_cycle_if_page_crossed(uint16_t base_addr, uint16_t offset, CPU::ADDRESSING_MODE mode, const AddrModeVec& modes);
        uint8_t     read_mem(uint16_t addr);
        void        write_mem(uint16_t addr, uint8_t data);
        void        push_stack(uint8_t data);
        uint8_t     pop_stack();

        // see https://web.archive.org/web/20200129081101/http://users.telenet.be:80/kim1-6502/6502/proman.html
        // for power-up sequence details
        void power_up();
        //void engage_reset();
        //void release_reset();

    public:
        void        write_brk_vec(uint16_t data);
        void        write_nmi_vec(uint16_t data);
        void        write_reset_vec(uint16_t data);
    private:
        uint16_t    read_nmi_vec();
        uint16_t    read_reset_vec();
        uint16_t    read_brk_vec();

        void raise_interrupt(bool maskable);
        void return_from_interrupt();


    public:
        enum OPCODE : uint8_t{
            /* LEGEND:
             * OPCODE_ADDRMODE = HEX // OPERATION, UPDATED FLAGS - ADDR MODE (if exclusive)
             */
            /* Load/Store */
            LDA_INDX = 0xA1,    // Load Accumulator 	N,Z
            LDA_ZPG  = 0xA5,    
            LDA_IMM  = 0xA9,    
            LDA_ABS  = 0xAD,    
            LDA_INDY = 0xB1,    
            LDA_ZPGX = 0xB5,    
            LDA_ABSY = 0xB9,    
            LDA_ABSX = 0xBD,    
            LDX_IMM  = 0xA2, 	// Load X Register 	    N,Z
            LDX_ZPG  = 0xA6, 	
            LDX_ABS  = 0xAE, 	
            LDX_ZPGY = 0xB6, 	
            LDX_ABSY = 0xBE, 	
            LDY_IMM  = 0xA0, 	// Load Y Register 	    N,Z
            LDY_ZPG  = 0xA4, 	
            LDY_ABS  = 0xAC, 	
            LDY_ZPGX = 0xB4, 	
            LDY_ABSX = 0xBC, 	
            STA_INDX = 0x81, 	// Store Accumulator 	 
            STA_ZPG  = 0x85, 	 	 
            STA_ABS  = 0x8D, 	 	 
            STA_INDY = 0x91, 	 	 
            STA_ZPGX = 0x95, 	 	 
            STA_ABSY = 0x99, 	 	 
            STA_ABSX = 0x9D, 	 	 
            STX_ZPG  = 0x86, 	// Store X Register 	 
            STX_ABS  = 0x8E, 	 	 
            STX_ZPGY = 0x96, 	 	 
            STY_ZPG  = 0x84, 	// Store Y Register 	 
            STY_ABS  = 0x8C, 	 	 
            STY_ZPGX = 0x94, 	 	 

            /* Register Transfers */
            TAX_IMPL = 0xAA, 	    // Transfer accumulator to X 	N,Z
            TAY_IMPL = 0xA8, 	    // Transfer accumulator to Y 	N,Z
            TXA_IMPL = 0x8A, 	    // Transfer X to accumulator 	N,Z
            TYA_IMPL = 0x98, 	    // Transfer Y to accumulator 	N,Z

            /* Stack Operations */
            TSX_IMPL = 0xBA, 	    // Transfer stack pointer to X 	    N,Z
            TXS_IMPL = 0x9A, 	    // Transfer X to stack pointer 	 
            PHA_IMPL = 0x48, 	    // Push accumulator on stack 	 
            PHP_IMPL = 0x08, 	    // Push processor status on stack 	 
            PLA_IMPL = 0x68, 	    // Pull accumulator from stack 	    N,Z
            PLP_IMPL = 0x28, 	    // Pull processor status from stack All

            /* Logical */
            AND_INDX = 0x21, 	    // Logical AND 	            N,Z
            AND_ZPG  = 0x25, 	    
            AND_IMM  = 0x29, 	    
            AND_ABS  = 0x2D, 	    
            AND_INDY = 0x31, 	    
            AND_ZPGX = 0x35, 	    
            AND_ABSY = 0x39, 	    
            AND_ABSX = 0x3D, 	    
            EOR_INDX = 0x41, 	    // Exclusive OR 	        N,Z
            EOR_ZPG  = 0x45, 	    
            EOR_IMM  = 0x49, 	    
            EOR_ABS  = 0x4D, 	    
            EOR_INDY = 0x51, 	    
            EOR_ZPGX = 0x55, 	    
            EOR_ABSY = 0x59, 	    
            EOR_ABSX = 0x5D, 	    
            ORA_INDX = 0x01, 	    // Logical Inclusive OR 	N,Z
            ORA_ZPG  = 0x05, 	    
            ORA_IMM  = 0x09, 	    
            ORA_ABS  = 0x0D, 	    
            ORA_INDY = 0x11, 	    
            ORA_ZPGX = 0x15, 	    
            ORA_ABSY = 0x19, 	    
            ORA_ABSX = 0x1D, 	    
            BIT_ZPG  = 0x24, 	    // Bit Test 	            N,V,Z
            BIT_ABS  = 0x2C, 	    

            /* Arithmetic */
            ADC_INDX = 0x61, 	    // Add with Carry 	    N,V,Z,C 
            ADC_ZPG  = 0x65, 	    
            ADC_IMM  = 0x69, 	    
            ADC_ABS  = 0x6D, 	    
            ADC_INDY = 0x71, 	    
            ADC_ZPGX = 0x75, 	    
            ADC_ABSY = 0x79, 	    
            ADC_ABSX = 0x7D, 	    
            SBC_INDX = 0xE1, 	    // Subtract with Carry 	N,V,Z,C
            SBC_ZPG  = 0xE5, 	    
            SBC_IMM  = 0xE9, 	    
            SBC_ABS  = 0xED, 	    
            SBC_INDY = 0xF1, 	    
            SBC_ZPGX = 0xF5, 	    
            SBC_ABSY = 0xF9, 	    
            SBC_ABSX = 0xFD, 	    
            CMP_INDX = 0xC1, 	    // Compare accumulator 	N,Z,C
            CMP_ZPG  = 0xC5, 	    
            CMP_IMM  = 0xC9, 	    
            CMP_ABS  = 0xCD, 	    
            CMP_INDY = 0xD1, 	    
            CMP_ZPGX = 0xD5, 	    
            CMP_ABSY = 0xD9, 	    
            CMP_ABSX = 0xDD, 	    
            CPX_IMM  = 0xE0, 	    // Compare X register 	N,Z,C
            CPX_ZPG  = 0xE4, 	    
            CPX_ABS  = 0xEC, 	    
            CPY_IMM  = 0xC0, 	    // Compare Y register 	N,Z,C
            CPY_ZPG  = 0xC4, 	    
            CPY_ABS  = 0xCC, 	    

            /* Increments & Decrements */
            INC_ZPG  = 0xE6, 	    // Increment a memory location 	N,Z
            INC_ABS  = 0xEE, 	    
            INC_ZPGX = 0xF6, 	    
            INC_ABSX = 0xFE, 	    
            INX_IMPL = 0xE8, 	    // Increment the X register 	N,Z
            INY_IMPL = 0xC8, 	    // Increment the Y register 	N,Z
            DEC_ZPG  = 0xC6, 	    // Decrement a memory location 	N,Z
            DEC_ABS  = 0xCE, 	    
            DEC_ZPGX = 0xD6, 	    
            DEC_ABSX = 0xDE, 	    
            DEX_IMPL = 0xCA, 	    // Decrement the X register 	N,Z
            DEY_IMPL = 0x88, 	    // Decrement the Y register 	N,Z

            /* Shifts */
            ASL_ZPG  = 0x06, 	    // Arithmetic Shift Left 	N,Z,C
            ASL_ACC  = 0x0A, 	    
            ASL_ABS  = 0x0E, 	    
            ASL_ZPGX = 0x16, 	    
            ASL_ABSX = 0x1E, 	    
            LSR_ZPG  = 0x46, 	    // Logical Shift Right 	    N,Z,C
            LSR_ACC  = 0x4A, 	    
            LSR_ABS  = 0x4E, 	    
            LSR_ZPGX = 0x56, 	    
            LSR_ABSX = 0x5E, 	    
            ROL_ZPG  = 0x26, 	    // Rotate Left 	            N,Z,C
            ROL_ACC  = 0x2A, 	    
            ROL_ABS  = 0x2E, 	    
            ROL_ZPGX = 0x36, 	    
            ROL_ABSX = 0x3E, 	    
            ROR_ZPG  = 0x66, 	    // Rotate Right 	        N,Z,C
            ROR_ACC  = 0x6A, 	    
            ROR_ABS  = 0x6E, 	    
            ROR_ZPGX = 0x76, 	    
            ROR_ABSX = 0x7E, 	    

            /* Jumps & Calls */
            JMP_ABS  = 0x4C, 	    // Jump to another location 	 
            JMP_IND  = 0x6C, 	     	 
            JSR_ABS  = 0x20, 	    // Jump to a subroutine 	 
            RTS_IMPL = 0x60, 	    // Return from subroutine 	 

            /* Branches */
            BCC_REL = 0x90, 	    // Branch if carry flag clear 	 
            BCS_REL = 0xB0, 	    // Branch if carry flag set 	 
            BEQ_REL = 0xF0, 	    // Branch if zero flag set 	 
            BMI_REL = 0x30, 	    // Branch if negative flag set 	 
            BNE_REL = 0xD0, 	    // Branch if zero flag clear 	 
            BPL_REL = 0x10, 	    // Branch if negative flag clear 	 
            BVC_REL = 0x50, 	    // Branch if overflow flag clear 	 
            BVS_REL = 0x70, 	    // Branch if overflow flag set 	 

            /* Status Flag Changes */
            CLC_IMPL = 0x18, 	    // Clear carry flag 	            C
            CLD_IMPL = 0xD8, 	    // Clear decimal mode flag 	        D
            CLI_IMPL = 0x58, 	    // Clear interrupt disable flag 	I
            CLV_IMPL = 0xB8, 	    // Clear overflow flag 	            V
            SEC_IMPL = 0x38, 	    // Set carry flag 	                C
            SED_IMPL = 0xF8, 	    // Set decimal mode flag 	        D
            SEI_IMPL = 0x78, 	    // Set interrupt disable flag 	    I

            /* System Functions */
            BRK_IMPL = 0x00, 	    // Force an interrupt 	    B
            NOP_IMPL = 0xEA, 	    // No Operation
            RTI_IMPL = 0x40,	    // Return from Interrupt 	All

            /* Unofficial/Illegal opcodes */
            /* Illegal NOP's */
            NOP_IMM_ILL0  = 0x80,
            NOP_IMM_ILL1  = 0x82,
            NOP_IMM_ILL2  = 0xC2,
            NOP_IMM_ILL3  = 0xE2,
            NOP_IMM_ILL4  = 0x89,
            NOP_ZPG_ILL0  = 0x04,
            NOP_ZPG_ILL1  = 0x44,
            NOP_ZPG_ILL2  = 0x64,
            NOP_ZPGX_ILL0 = 0x14,
            NOP_ZPGX_ILL1 = 0x34,
            NOP_ZPGX_ILL2 = 0x54,
            NOP_ZPGX_ILL3 = 0x74,
            NOP_ZPGX_ILL4 = 0xD4,
            NOP_ZPGX_ILL5 = 0xF4,
            NOP_IMPL_ILL0 = 0x1A,
            NOP_IMPL_ILL1 = 0x3A,
            NOP_IMPL_ILL2 = 0x5A,
            NOP_IMPL_ILL3 = 0x7A,
            NOP_IMPL_ILL4 = 0xDA,
            NOP_IMPL_ILL5 = 0xFA,
            NOP_ABS_ILL   = 0x0C,
            NOP_ABSX_ILL0 = 0x1C,
            NOP_ABSX_ILL1 = 0x3C,
            NOP_ABSX_ILL2 = 0x5C,
            NOP_ABSX_ILL3 = 0x7C,
            NOP_ABSX_ILL4 = 0xDC,
            NOP_ABSX_ILL5 = 0xFC,

            /* JAM instructions cause the CPU to loop/halt indefinitely */
            JAM_IMPL_ILL0  = 0x02,
            JAM_IMPL_ILL1  = 0x12,
            JAM_IMPL_ILL2  = 0x22,
            JAM_IMPL_ILL3  = 0x32,
            JAM_IMPL_ILL4  = 0x42,
            JAM_IMPL_ILL5  = 0x52,
            JAM_IMPL_ILL6  = 0x62,
            JAM_IMPL_ILL7  = 0x72,
            JAM_IMPL_ILL8  = 0x92,
            JAM_IMPL_ILL9  = 0xB2,
            JAM_IMPL_ILL10 = 0xD2,
            JAM_IMPL_ILL11 = 0xF2,

            // TODO: compile remaining illegal opcodes 
            /* SLO = ASL combined with ORA */
            SLO_INDX_ILL = 0x03,
            SLO_INDY_ILL = 0x13,
            SLO_ZPG_ILL  = 0x07,
            SLO_ZPGX_ILL = 0x17,
            SLO_ABSY_ILL = 0x1B,
            SLO_ABS_ILL  = 0x0F,
            SLO_ABSX_ILL = 0x1F,

            /* RLA = AND combined with ROL */
            RLA_INDX_ILL = 0x23,
            RLA_INDY_ILL = 0x33,
            RLA_ZPG_ILL  = 0x27,
            RLA_ZPGX_ILL = 0x37,
            RLA_ABSY_ILL = 0x3B,
            RLA_ABS_ILL  = 0x2F,
            RLA_ABSX_ILL = 0x3F,

            /* SRE = LSR combined with EOR */
            SRE_INDX_ILL = 0x43,
            SRE_INDY_ILL = 0x53,
            SRE_ZPG_ILL  = 0x47,
            SRE_ZPGX_ILL = 0x57,
            SRE_ABSY_ILL = 0x5B,
            SRE_ABS_ILL  = 0x4F,
            SRE_ABSX_ILL = 0x5F,

            /* RRA = ROR combined with ADC */
            RRA_INDX_ILL = 0x63,
            RRA_INDY_ILL = 0x73,
            RRA_ZPG_ILL  = 0x67,
            RRA_ZPGX_ILL = 0x77,
            RRA_ABSY_ILL = 0x7B,
            RRA_ABS_ILL  = 0x6F,
            RRA_ABSX_ILL = 0x7F,

            /* SAX */
            SAX_INDX_ILL = 0x83,
            SAX_ZPG_ILL = 0x87,
            SAX_ZPGY_ILL = 0x97,
            SAX_ABS_ILL = 0x8F,

            /* LAX = LDA combined with LDX */
            LAX_INDX_ILL = 0xA3,
            LAX_INDY_ILL = 0xB3,
            LAX_ZPG_ILL  = 0xA7,
            LAX_ZPGY_ILL = 0xB7,
            LAX_ABS_ILL  = 0xAF,
            LAX_ABSY_ILL = 0xBF,

            /* DCP = LDA combined with TSX */
            DCP_INDX_ILL = 0xC3,
            DCP_INDY_ILL = 0xD3,
            DCP_ZPG_ILL  = 0xC7,
            DCP_ZPGX_ILL = 0xD7,
            DCP_ABSY_ILL = 0xDB,
            DCP_ABS_ILL  = 0xCF,
            DCP_ABSX_ILL = 0xDF,

            /* ISC = INC combined with SBC */
            ISC_INDX_ILL = 0xE3,
            ISC_INDY_ILL = 0xF3,
            ISC_ZPG_ILL  = 0xE7,
            ISC_ZPGX_ILL = 0xF7,
            ISC_ABSY_ILL = 0xFB,
            ISC_ABS_ILL  = 0xEF,
            ISC_ABSX_ILL = 0xFF,
            
            /* ANC = AND combined with set C */
            ANC_IMM_ILL0 = 0x0B,
            ANC_IMM_ILL1 = 0x2B,

            // misc
            ALR_IMM_ILL  = 0x4B,    // ALR = AND combined with LSR
            ARR_IMM_ILL  = 0x6B,    // ARR = AND combined with ROR
            ANE_IMM_ILL  = 0x8B,    // ANE = ANDX combined with AND
            SHA_INDY_ILL = 0x93,
            SHA_ABSY_ILL = 0x9F,
            SHY_ABSX_ILL = 0x9C,
            SHX_ABSY_ILL = 0x9E,
            TAS_ABSY_ILL = 0x9B,
            LXA_IMM_ILL  = 0xAB,
            LAS_ABSY_ILL = 0xBB,
            SBX_IMM_ILL  = 0xCB,
            USBC_IMM_ILL = 0xEB
        };

    private:
        /* INSTRUCTIONS */
        /* Load/Store */
        void LDA(enum ADDRESSING_MODE mode);    // Load Accumulator 	N,Z
        void LDX(enum ADDRESSING_MODE mode); 	// Load X Register 	    N,Z
        void LDY(enum ADDRESSING_MODE mode); 	// Load Y Register 	    N,Z
        void STA(enum ADDRESSING_MODE mode); 	// Store Accumulator 	 
        void STX(enum ADDRESSING_MODE mode); 	// Store X Register 	 
        void STY(enum ADDRESSING_MODE mode);    // Store Y Register 	 

        /* Register Transfers */
        void TAX(); 	// Transfer accumulator to X 	N,Z - IMPLIED MODE ONLY
        void TAY(); 	// Transfer accumulator to Y 	N,Z - IMPLIED MODE ONLY
        void TXA(); 	// Transfer X to accumulator 	N,Z - IMPLIED MODE ONLY
        void TYA(); 	// Transfer Y to accumulator 	N,Z - IMPLIED MODE ONLY

        /* Stack Operations */
        void TSX(); 	// Transfer stack pointer to X 	    N,Z - IMPLIED MODE ONLY
        void TXS(); 	// Transfer X to stack pointer 	        - IMPLIED MODE ONLY 
        void PHA(); 	// Push accumulator on stack 	        - IMPLIED MODE ONLY 
        void PHP(); 	// Push processor status on stack 	    - IMPLIED MODE ONLY  
        void PLA(); 	// Pull accumulator from stack 	    N,Z - IMPLIED MODE ONLY
        void PLP(); 	// Pull processor status from stack All - IMPLIED MODE ONLY

        /* Logical */
        void AND(enum ADDRESSING_MODE mode); 	// Logical AND 	            N,Z
        void EOR(enum ADDRESSING_MODE mode); 	// Exclusive OR 	        N,Z
        void ORA(enum ADDRESSING_MODE mode); 	// Logical Inclusive OR 	N,Z
        void BIT(enum ADDRESSING_MODE mode); 	// Bit Test 	            N,V,Z

        /* Arithmetic */
        void ADC(enum ADDRESSING_MODE mode); 	// Add with Carry 	    N,V,Z,C 
        void SBC(enum ADDRESSING_MODE mode); 	// Subtract with Carry 	N,V,Z,C
        void CMP(enum ADDRESSING_MODE mode); 	// Compare accumulator 	N,Z,C
        void CPX(enum ADDRESSING_MODE mode); 	// Compare X register 	N,Z,C
        void CPY(enum ADDRESSING_MODE mode); 	// Compare Y register 	N,Z,C

        /* Increments & Decrements */
        void INC(enum ADDRESSING_MODE mode); 	// Increment a memory location 	N,Z
        void INX(); 	                        // Increment the X register 	N,Z - IMPLIED MODE ONLY
        void INY(); 	                        // Increment the Y register 	N,Z - IMPLIED MODE ONLZ
        void DEC(enum ADDRESSING_MODE mode); 	// Decrement a memory location 	N,Z
        void DEX(); 	                        // Decrement the X register 	N,Z - IMPLIED MODE ONLY
        void DEY(); 	                        // Decrement the Y register 	N,Z - IMPLIED MODE ONLY

        /* Shifts */
        void ASL(enum ADDRESSING_MODE mode); 	// Arithmetic Shift Left 	N,Z,C
        void ASL_eACC(); 	                    // ASL in Accumulator mode 	N,Z,C
        void LSR(enum ADDRESSING_MODE mode); 	// Logical Shift Right 	    N,Z,C
        void LSR_eACC();                        // LSR in Accumulator mode N,Z,C
        void ROL(enum ADDRESSING_MODE mode); 	// Rotate Left 	            N,Z,C
        void ROL_eACC();                        // ROL in Accumulator mode N,Z,C
        void ROR(enum ADDRESSING_MODE mode); 	// Rotate Right 	        N,Z,C
        void ROR_eACC();                        // ROR in Accumulator mode N,Z,C

        /* Jumps & Calls */
        void JMP(enum ADDRESSING_MODE mode);    // Jump to another location 	 
        void JSR(); 	                        // Jump to a subroutine 	 - ABSOLUTE MODE ONLY
        void RTS(); 	                        // Return from subroutine 	 - IMPLIED MODE ONLY

        /* Branches */
        void branch(bit condition); // helper function for generic branch behaviour
        void BCC(); 	// Branch if carry flag clear    - RELATIVE MODE ONLY
        void BCS(); 	// Branch if carry flag set      - RELATIVE MODE ONLY
        void BEQ(); 	// Branch if zero flag set 	     - RELATIVE MODE ONLY
        void BMI(); 	// Branch if negative flag set 	 - RELATIVE MODE ONLY
        void BNE(); 	// Branch if zero flag clear 	 - RELATIVE MODE ONLY
        void BPL(); 	// Branch if negative flag clear - RELATIVE MODE ONLY	 
        void BVC(); 	// Branch if overflow flag clear - RELATIVE MODE ONLY	 
        void BVS(); 	// Branch if overflow flag set 	 - RELATIVE MODE ONLY

        /* Status Flag Changes */
        void CLC(); 	// Clear carry flag 	            C - IMPLIED MODE ONLY
        void CLD(); 	// Clear decimal mode flag 	        D - IMPLIED MODE ONLY
        void CLI(); 	// Clear interrupt disable flag 	I - IMPLIED MODE ONLY
        void CLV(); 	// Clear overflow flag 	            V - IMPLIED MODE ONLY
        void SEC(); 	// Set carry flag 	                C - IMPLIED MODE ONLY
        void SED(); 	// Set decimal mode flag 	        D - IMPLIED MODE ONLY
        void SEI(); 	// Set interrupt disable flag 	    I - IMPLIED MODE ONLY

        /* System Functions */
        void BRK(); 	// Force an interrupt 	    B - IMPLIED MODE ONLY
        void NOP(); 	// No Operation (illegal nop's also exist) - IMPLIED MODE ONLY
        void RTI();	    // Return from Interrupt 	All - IMPLIED MODE ONLY

        /* Unofficial/Illegal opcodes */
        void NOP_ILL(); // Illegal NOP 
        void JAM_ILL(); // JAM causes the CPU to loop/halt indefinitely
        void SLO_ILL(); // SLO = ASL combined with ORA 
        void RLA_ILL(); // RLA = AND combined with ROL 
        void SRE_ILL(); // SRE = LSR combined with EOR 
        void RRA_ILL(); // RRA = ROR combined with ADC 
        void SAX_ILL(); // SAX 
        void LAX_ILL(); // LAX = LDA combined with LDX 
        void DCP_ILL(); // DCP = LDA combined with TSX 
        void ISC_ILL(); // ISC = INC combined with SBC 
        void ANC_ILL(); // ANC = AND combined with set C 
        void ALR_ILL(); // ALR = AND combined with LSR 
        void ARR_ILL(); // ARR = AND combined with ROR 
        void ANE_ILL(); // ANE = ANDX combined with AND 
        void SHA_ILL();
        void SHY_ILL();
        void SHX_ILL();
        void TAS_ILL();
        void LXA_ILL();
        void LAS_ILL();
        void SBX_ILL();
        void USBC_ILL();

        // allow opcode enums to have their name printed
    public:
        friend std::ostream& operator<<(std::ostream& out, const enum OPCODE value);

};

// make ADDRESSING_MODE enums printable
std::ostream& operator<<(std::ostream& out, const CPU::OPCODE value){
    // wow this sucks, but whatever it works and its not very important
    std::string str {};
    switch(value){
        case CPU::LDA_INDX: str = std::string("LDA_INDX"); return out << str;
        case CPU::LDA_ZPG: str = std::string("LDA_ZPG"); return out << str;
        case CPU::LDA_IMM: str = std::string("LDA_IMM"); return out << str;
        case CPU::LDA_ABS: str = std::string("LDA_ABS"); return out << str;
        case CPU::LDA_INDY: str = std::string("LDA_INDY"); return out << str;
        case CPU::LDA_ZPGX: str = std::string("LDA_ZPGX"); return out << str;
        case CPU::LDA_ABSY: str = std::string("LDA_ABSY"); return out << str;
        case CPU::LDA_ABSX: str = std::string("LDA_ABSX"); return out << str;
        case CPU::LDX_IMM: str = std::string("LDX_IMM"); return out << str;
        case CPU::LDX_ZPG: str = std::string("LDX_ZPG"); return out << str;
        case CPU::LDX_ABS: str = std::string("LDX_ABS"); return out << str;
        case CPU::LDX_ZPGY: str = std::string("LDX_ZPGY"); return out << str;
        case CPU::LDX_ABSY: str = std::string("LDX_ABSY"); return out << str;
        case CPU::LDY_IMM: str = std::string("LDY_IMM"); return out << str;
        case CPU::LDY_ZPG: str = std::string("LDY_ZPG"); return out << str;
        case CPU::LDY_ABS: str = std::string("LDY_ABS"); return out << str;
        case CPU::LDY_ZPGX: str = std::string("LDY_ZPGX"); return out << str;
        case CPU::LDY_ABSX: str = std::string("LDY_ABSX"); return out << str;
        case CPU::STA_INDX: str = std::string("STA_INDX"); return out << str;
        case CPU::STA_ZPG: str = std::string("STA_ZPG"); return out << str;
        case CPU::STA_ABS: str = std::string("STA_ABS"); return out << str;
        case CPU::STA_INDY: str = std::string("STA_INDY"); return out << str;
        case CPU::STA_ZPGX: str = std::string("STA_ZPGX"); return out << str;
        case CPU::STA_ABSY: str = std::string("STA_ABSY"); return out << str;
        case CPU::STA_ABSX: str = std::string("STA_ABSX"); return out << str;
        case CPU::STX_ZPG: str = std::string("STX_ZPG"); return out << str;
        case CPU::STX_ABS: str = std::string("STX_ABS"); return out << str;
        case CPU::STX_ZPGY: str = std::string("STX_ZPGY"); return out << str;
        case CPU::STY_ZPG: str = std::string("STY_ZPG"); return out << str;
        case CPU::STY_ABS: str = std::string("STY_ABS"); return out << str;
        case CPU::STY_ZPGX: str = std::string("STY_ZPGX"); return out << str;
        case CPU::TAX_IMPL: str = std::string("TAX_IMPL"); return out << str;
        case CPU::TAY_IMPL: str = std::string("TAY_IMPL"); return out << str;
        case CPU::TXA_IMPL: str = std::string("TXA_IMPL"); return out << str;
        case CPU::TYA_IMPL: str = std::string("TYA_IMPL"); return out << str;
        case CPU::TSX_IMPL: str = std::string("TSX_IMPL"); return out << str;
        case CPU::TXS_IMPL: str = std::string("TXS_IMPL"); return out << str;
        case CPU::PHA_IMPL: str = std::string("PHA_IMPL"); return out << str;
        case CPU::PHP_IMPL: str = std::string("PHP_IMPL"); return out << str;
        case CPU::PLA_IMPL: str = std::string("PLA_IMPL"); return out << str;
        case CPU::PLP_IMPL: str = std::string("PLP_IMPL"); return out << str;
        case CPU::AND_INDX: str = std::string("AND_INDX"); return out << str;
        case CPU::AND_ZPG: str = std::string("AND_ZPG"); return out << str;
        case CPU::AND_IMM: str = std::string("AND_IMM"); return out << str;
        case CPU::AND_ABS: str = std::string("AND_ABS"); return out << str;
        case CPU::AND_INDY: str = std::string("AND_INDY"); return out << str;
        case CPU::AND_ZPGX: str = std::string("AND_ZPGX"); return out << str;
        case CPU::AND_ABSY: str = std::string("AND_ABSY"); return out << str;
        case CPU::AND_ABSX: str = std::string("AND_ABSX"); return out << str;
        case CPU::EOR_INDX: str = std::string("EOR_INDX"); return out << str;
        case CPU::EOR_ZPG: str = std::string("EOR_ZPG"); return out << str;
        case CPU::EOR_IMM: str = std::string("EOR_IMM"); return out << str;
        case CPU::EOR_ABS: str = std::string("EOR_ABS"); return out << str;
        case CPU::EOR_INDY: str = std::string("EOR_INDY"); return out << str;
        case CPU::EOR_ZPGX: str = std::string("EOR_ZPGX"); return out << str;
        case CPU::EOR_ABSY: str = std::string("EOR_ABSY"); return out << str;
        case CPU::EOR_ABSX: str = std::string("EOR_ABSX"); return out << str;
        case CPU::ORA_INDX: str = std::string("ORA_INDX"); return out << str;
        case CPU::ORA_ZPG: str = std::string("ORA_ZPG"); return out << str;
        case CPU::ORA_IMM: str = std::string("ORA_IMM"); return out << str;
        case CPU::ORA_ABS: str = std::string("ORA_ABS"); return out << str;
        case CPU::ORA_INDY: str = std::string("ORA_INDY"); return out << str;
        case CPU::ORA_ZPGX: str = std::string("ORA_ZPGX"); return out << str;
        case CPU::ORA_ABSY: str = std::string("ORA_ABSY"); return out << str;
        case CPU::ORA_ABSX: str = std::string("ORA_ABSX"); return out << str;
        case CPU::BIT_ZPG: str = std::string("BIT_ZPG"); return out << str;
        case CPU::BIT_ABS: str = std::string("BIT_ABS"); return out << str;
        case CPU::ADC_INDX: str = std::string("ADC_INDX"); return out << str;
        case CPU::ADC_ZPG: str = std::string("ADC_ZPG"); return out << str;
        case CPU::ADC_IMM: str = std::string("ADC_IMM"); return out << str;
        case CPU::ADC_ABS: str = std::string("ADC_ABS"); return out << str;
        case CPU::ADC_INDY: str = std::string("ADC_INDY"); return out << str;
        case CPU::ADC_ZPGX: str = std::string("ADC_ZPGX"); return out << str;
        case CPU::ADC_ABSY: str = std::string("ADC_ABSY"); return out << str;
        case CPU::ADC_ABSX: str = std::string("ADC_ABSX"); return out << str;
        case CPU::SBC_INDX: str = std::string("SBC_INDX"); return out << str;
        case CPU::SBC_ZPG: str = std::string("SBC_ZPG"); return out << str;
        case CPU::SBC_IMM: str = std::string("SBC_IMM"); return out << str;
        case CPU::SBC_ABS: str = std::string("SBC_ABS"); return out << str;
        case CPU::SBC_INDY: str = std::string("SBC_INDY"); return out << str;
        case CPU::SBC_ZPGX: str = std::string("SBC_ZPGX"); return out << str;
        case CPU::SBC_ABSY: str = std::string("SBC_ABSY"); return out << str;
        case CPU::SBC_ABSX: str = std::string("SBC_ABSX"); return out << str;
        case CPU::CMP_INDX: str = std::string("CMP_INDX"); return out << str;
        case CPU::CMP_ZPG: str = std::string("CMP_ZPG"); return out << str;
        case CPU::CMP_IMM: str = std::string("CMP_IMM"); return out << str;
        case CPU::CMP_ABS: str = std::string("CMP_ABS"); return out << str;
        case CPU::CMP_INDY: str = std::string("CMP_INDY"); return out << str;
        case CPU::CMP_ZPGX: str = std::string("CMP_ZPGX"); return out << str;
        case CPU::CMP_ABSY: str = std::string("CMP_ABSY"); return out << str;
        case CPU::CMP_ABSX: str = std::string("CMP_ABSX"); return out << str;
        case CPU::CPX_IMM: str = std::string("CPX_IMM"); return out << str;
        case CPU::CPX_ZPG: str = std::string("CPX_ZPG"); return out << str;
        case CPU::CPX_ABS: str = std::string("CPX_ABS"); return out << str;
        case CPU::CPY_IMM: str = std::string("CPY_IMM"); return out << str;
        case CPU::CPY_ZPG: str = std::string("CPY_ZPG"); return out << str;
        case CPU::CPY_ABS: str = std::string("CPY_ABS"); return out << str;
        case CPU::INC_ZPG: str = std::string("INC_ZPG"); return out << str;
        case CPU::INC_ABS: str = std::string("INC_ABS"); return out << str;
        case CPU::INC_ZPGX: str = std::string("INC_ZPGX"); return out << str;
        case CPU::INC_ABSX: str = std::string("INC_ABSX"); return out << str;
        case CPU::INX_IMPL: str = std::string("INX_IMPL"); return out << str;
        case CPU::INY_IMPL: str = std::string("INY_IMPL"); return out << str;
        case CPU::DEC_ZPG: str = std::string("DEC_ZPG"); return out << str;
        case CPU::DEC_ABS: str = std::string("DEC_ABS"); return out << str;
        case CPU::DEC_ZPGX: str = std::string("DEC_ZPGX"); return out << str;
        case CPU::DEC_ABSX: str = std::string("DEC_ABSX"); return out << str;
        case CPU::DEX_IMPL: str = std::string("DEX_IMPL"); return out << str;
        case CPU::DEY_IMPL: str = std::string("DEY_IMPL"); return out << str;
        case CPU::ASL_ZPG: str = std::string("ASL_ZPG"); return out << str;
        case CPU::ASL_ACC: str = std::string("ASL_ACC"); return out << str;
        case CPU::ASL_ABS: str = std::string("ASL_ABS"); return out << str;
        case CPU::ASL_ZPGX: str = std::string("ASL_ZPGX"); return out << str;
        case CPU::ASL_ABSX: str = std::string("ASL_ABSX"); return out << str;
        case CPU::LSR_ZPG: str = std::string("LSR_ZPG"); return out << str;
        case CPU::LSR_ACC: str = std::string("LSR_ACC"); return out << str;
        case CPU::LSR_ABS: str = std::string("LSR_ABS"); return out << str;
        case CPU::LSR_ZPGX: str = std::string("LSR_ZPGX"); return out << str;
        case CPU::LSR_ABSX: str = std::string("LSR_ABSX"); return out << str;
        case CPU::ROL_ZPG: str = std::string("ROL_ZPG"); return out << str;
        case CPU::ROL_ACC: str = std::string("ROL_ACC"); return out << str;
        case CPU::ROL_ABS: str = std::string("ROL_ABS"); return out << str;
        case CPU::ROL_ZPGX: str = std::string("ROL_ZPGX"); return out << str;
        case CPU::ROL_ABSX: str = std::string("ROL_ABSX"); return out << str;
        case CPU::ROR_ZPG: str = std::string("ROR_ZPG"); return out << str;
        case CPU::ROR_ACC: str = std::string("ROR_ACC"); return out << str;
        case CPU::ROR_ABS: str = std::string("ROR_ABS"); return out << str;
        case CPU::ROR_ZPGX: str = std::string("ROR_ZPGX"); return out << str;
        case CPU::ROR_ABSX: str = std::string("ROR_ABSX"); return out << str;
        case CPU::JMP_ABS: str = std::string("JMP_ABS"); return out << str;
        case CPU::JMP_IND: str = std::string("JMP_IND"); return out << str;
        case CPU::JSR_ABS: str = std::string("JSR_ABS"); return out << str;
        case CPU::RTS_IMPL: str = std::string("RTS_IMPL"); return out << str;
        case CPU::BCC_REL: str = std::string("BCC_REL"); return out << str;
        case CPU::BCS_REL: str = std::string("BCS_REL"); return out << str;
        case CPU::BEQ_REL: str = std::string("BEQ_REL"); return out << str;
        case CPU::BMI_REL: str = std::string("BMI_REL"); return out << str;
        case CPU::BNE_REL: str = std::string("BNE_REL"); return out << str;
        case CPU::BPL_REL: str = std::string("BPL_REL"); return out << str;
        case CPU::BVC_REL: str = std::string("BVC_REL"); return out << str;
        case CPU::BVS_REL: str = std::string("BVS_REL"); return out << str;
        case CPU::CLC_IMPL: str = std::string("CLC_IMPL"); return out << str;
        case CPU::CLD_IMPL: str = std::string("CLD_IMPL"); return out << str;
        case CPU::CLI_IMPL: str = std::string("CLI_IMPL"); return out << str;
        case CPU::CLV_IMPL: str = std::string("CLV_IMPL"); return out << str;
        case CPU::SEC_IMPL: str = std::string("SEC_IMPL"); return out << str;
        case CPU::SED_IMPL: str = std::string("SED_IMPL"); return out << str;
        case CPU::SEI_IMPL: str = std::string("SEI_IMPL"); return out << str;
        case CPU::BRK_IMPL: str = std::string("BRK_IMPL"); return out << str;
        case CPU::NOP_IMPL: str = std::string("NOP_IMPL"); return out << str;
        case CPU::RTI_IMPL: str = std::string("RTI_IMPL"); return out << str;
        case CPU::NOP_IMM_ILL0: str = std::string("NOP_IMM_ILL0"); return out << str;
        case CPU::NOP_IMM_ILL1: str = std::string("NOP_IMM_ILL1"); return out << str;
        case CPU::NOP_IMM_ILL2: str = std::string("NOP_IMM_ILL2"); return out << str;
        case CPU::NOP_IMM_ILL3: str = std::string("NOP_IMM_ILL3"); return out << str;
        case CPU::NOP_IMM_ILL4: str = std::string("NOP_IMM_ILL4"); return out << str;
        case CPU::NOP_ZPG_ILL0: str = std::string("NOP_ZPG_ILL0"); return out << str;
        case CPU::NOP_ZPG_ILL1: str = std::string("NOP_ZPG_ILL1"); return out << str;
        case CPU::NOP_ZPG_ILL2: str = std::string("NOP_ZPG_ILL2"); return out << str;
        case CPU::NOP_ZPGX_ILL0: str = std::string("NOP_ZPGX_ILL0"); return out << str;
        case CPU::NOP_ZPGX_ILL1: str = std::string("NOP_ZPGX_ILL1"); return out << str;
        case CPU::NOP_ZPGX_ILL2: str = std::string("NOP_ZPGX_ILL2"); return out << str;
        case CPU::NOP_ZPGX_ILL3: str = std::string("NOP_ZPGX_ILL3"); return out << str;
        case CPU::NOP_ZPGX_ILL4: str = std::string("NOP_ZPGX_ILL4"); return out << str;
        case CPU::NOP_ZPGX_ILL5: str = std::string("NOP_ZPGX_ILL5"); return out << str;
        case CPU::NOP_IMPL_ILL0: str = std::string("NOP_IMPL_ILL0"); return out << str;
        case CPU::NOP_IMPL_ILL1: str = std::string("NOP_IMPL_ILL1"); return out << str;
        case CPU::NOP_IMPL_ILL2: str = std::string("NOP_IMPL_ILL2"); return out << str;
        case CPU::NOP_IMPL_ILL3: str = std::string("NOP_IMPL_ILL3"); return out << str;
        case CPU::NOP_IMPL_ILL4: str = std::string("NOP_IMPL_ILL4"); return out << str;
        case CPU::NOP_IMPL_ILL5: str = std::string("NOP_IMPL_ILL5"); return out << str;
        case CPU::NOP_ABS_ILL : str = std::string("NOP_ABS_ILL "); return out << str;
        case CPU::NOP_ABSX_ILL0: str = std::string("NOP_ABSX_ILL0"); return out << str;
        case CPU::NOP_ABSX_ILL1: str = std::string("NOP_ABSX_ILL1"); return out << str;
        case CPU::NOP_ABSX_ILL2: str = std::string("NOP_ABSX_ILL2"); return out << str;
        case CPU::NOP_ABSX_ILL3: str = std::string("NOP_ABSX_ILL3"); return out << str;
        case CPU::NOP_ABSX_ILL4: str = std::string("NOP_ABSX_ILL4"); return out << str;
        case CPU::NOP_ABSX_ILL5: str = std::string("NOP_ABSX_ILL5"); return out << str;
        case CPU::JAM_IMPL_ILL0: str = std::string("JAM_IMPL_ILL0"); return out << str;
        case CPU::JAM_IMPL_ILL1: str = std::string("JAM_IMPL_ILL1"); return out << str;
        case CPU::JAM_IMPL_ILL2: str = std::string("JAM_IMPL_ILL2"); return out << str;
        case CPU::JAM_IMPL_ILL3: str = std::string("JAM_IMPL_ILL3"); return out << str;
        case CPU::JAM_IMPL_ILL4: str = std::string("JAM_IMPL_ILL4"); return out << str;
        case CPU::JAM_IMPL_ILL5: str = std::string("JAM_IMPL_ILL5"); return out << str;
        case CPU::JAM_IMPL_ILL6: str = std::string("JAM_IMPL_ILL6"); return out << str;
        case CPU::JAM_IMPL_ILL7: str = std::string("JAM_IMPL_ILL7"); return out << str;
        case CPU::JAM_IMPL_ILL8: str = std::string("JAM_IMPL_ILL8"); return out << str;
        case CPU::JAM_IMPL_ILL9: str = std::string("JAM_IMPL_ILL9"); return out << str;
        case CPU::JAM_IMPL_ILL10: str = std::string("JAM_IMPL_ILL10"); return out << str;
        case CPU::JAM_IMPL_ILL11: str = std::string("JAM_IMPL_ILL11"); return out << str;
        case CPU::SLO_INDX_ILL: str = std::string("SLO_INDX_ILL"); return out << str;
        case CPU::SLO_INDY_ILL: str = std::string("SLO_INDY_ILL"); return out << str;
        case CPU::SLO_ZPG_ILL: str = std::string("SLO_ZPG_ILL"); return out << str;
        case CPU::SLO_ZPGX_ILL: str = std::string("SLO_ZPGX_ILL"); return out << str;
        case CPU::SLO_ABSY_ILL: str = std::string("SLO_ABSY_ILL"); return out << str;
        case CPU::SLO_ABS_ILL: str = std::string("SLO_ABS_ILL"); return out << str;
        case CPU::SLO_ABSX_ILL: str = std::string("SLO_ABSX_ILL"); return out << str;
        case CPU::RLA_INDX_ILL: str = std::string("RLA_INDX_ILL"); return out << str;
        case CPU::RLA_INDY_ILL: str = std::string("RLA_INDY_ILL"); return out << str;
        case CPU::RLA_ZPG_ILL: str = std::string("RLA_ZPG_ILL"); return out << str;
        case CPU::RLA_ZPGX_ILL: str = std::string("RLA_ZPGX_ILL"); return out << str;
        case CPU::RLA_ABSY_ILL: str = std::string("RLA_ABSY_ILL"); return out << str;
        case CPU::RLA_ABS_ILL: str = std::string("RLA_ABS_ILL"); return out << str;
        case CPU::RLA_ABSX_ILL: str = std::string("RLA_ABSX_ILL"); return out << str;
        case CPU::SRE_INDX_ILL: str = std::string("SRE_INDX_ILL"); return out << str;
        case CPU::SRE_INDY_ILL: str = std::string("SRE_INDY_ILL"); return out << str;
        case CPU::SRE_ZPG_ILL: str = std::string("SRE_ZPG_ILL"); return out << str;
        case CPU::SRE_ZPGX_ILL: str = std::string("SRE_ZPGX_ILL"); return out << str;
        case CPU::SRE_ABSY_ILL: str = std::string("SRE_ABSY_ILL"); return out << str;
        case CPU::SRE_ABS_ILL: str = std::string("SRE_ABS_ILL"); return out << str;
        case CPU::SRE_ABSX_ILL: str = std::string("SRE_ABSX_ILL"); return out << str;
        case CPU::RRA_INDX_ILL: str = std::string("RRA_INDX_ILL"); return out << str;
        case CPU::RRA_INDY_ILL: str = std::string("RRA_INDY_ILL"); return out << str;
        case CPU::RRA_ZPG_ILL: str = std::string("RRA_ZPG_ILL"); return out << str;
        case CPU::RRA_ZPGX_ILL: str = std::string("RRA_ZPGX_ILL"); return out << str;
        case CPU::RRA_ABSY_ILL: str = std::string("RRA_ABSY_ILL"); return out << str;
        case CPU::RRA_ABS_ILL: str = std::string("RRA_ABS_ILL"); return out << str;
        case CPU::RRA_ABSX_ILL: str = std::string("RRA_ABSX_ILL"); return out << str;
        case CPU::SAX_INDX_ILL: str = std::string("SAX_INDX_ILL"); return out << str;
        case CPU::SAX_ZPG_ILL: str = std::string("SAX_ZPG_ILL"); return out << str;
        case CPU::SAX_ZPGY_ILL: str = std::string("SAX_ZPGY_ILL"); return out << str;
        case CPU::SAX_ABS_ILL: str = std::string("SAX_ABS_ILL"); return out << str;
        case CPU::LAX_INDX_ILL: str = std::string("LAX_INDX_ILL"); return out << str;
        case CPU::LAX_INDY_ILL: str = std::string("LAX_INDY_ILL"); return out << str;
        case CPU::LAX_ZPG_ILL: str = std::string("LAX_ZPG_ILL"); return out << str;
        case CPU::LAX_ZPGY_ILL: str = std::string("LAX_ZPGY_ILL"); return out << str;
        case CPU::LAX_ABS_ILL: str = std::string("LAX_ABS_ILL"); return out << str;
        case CPU::LAX_ABSY_ILL: str = std::string("LAX_ABSY_ILL"); return out << str;
        case CPU::DCP_INDX_ILL: str = std::string("DCP_INDX_ILL"); return out << str;
        case CPU::DCP_INDY_ILL: str = std::string("DCP_INDY_ILL"); return out << str;
        case CPU::DCP_ZPG_ILL: str = std::string("DCP_ZPG_ILL"); return out << str;
        case CPU::DCP_ZPGX_ILL: str = std::string("DCP_ZPGX_ILL"); return out << str;
        case CPU::DCP_ABSY_ILL: str = std::string("DCP_ABSY_ILL"); return out << str;
        case CPU::DCP_ABS_ILL: str = std::string("DCP_ABS_ILL"); return out << str;
        case CPU::DCP_ABSX_ILL: str = std::string("DCP_ABSX_ILL"); return out << str;
        case CPU::ISC_INDX_ILL: str = std::string("ISC_INDX_ILL"); return out << str;
        case CPU::ISC_INDY_ILL: str = std::string("ISC_INDY_ILL"); return out << str;
        case CPU::ISC_ZPG_ILL: str = std::string("ISC_ZPG_ILL"); return out << str;
        case CPU::ISC_ZPGX_ILL: str = std::string("ISC_ZPGX_ILL"); return out << str;
        case CPU::ISC_ABSY_ILL: str = std::string("ISC_ABSY_ILL"); return out << str;
        case CPU::ISC_ABS_ILL: str = std::string("ISC_ABS_ILL"); return out << str;
        case CPU::ISC_ABSX_ILL: str = std::string("ISC_ABSX_ILL"); return out << str;
        case CPU::ANC_IMM_ILL0: str = std::string("ANC_IMM_ILL0"); return out << str;
        case CPU::ANC_IMM_ILL1: str = std::string("ANC_IMM_ILL1"); return out << str;
        case CPU::ALR_IMM_ILL: str = std::string("ALR_IMM_ILL"); return out << str;
        case CPU::ARR_IMM_ILL: str = std::string("ARR_IMM_ILL"); return out << str;
        case CPU::ANE_IMM_ILL: str = std::string("ANE_IMM_ILL"); return out << str;
        case CPU::SHA_INDY_ILL: str = std::string("SHA_INDY_ILL"); return out << str;
        case CPU::SHA_ABSY_ILL: str = std::string("SHA_ABSY_ILL"); return out << str;
        case CPU::SHY_ABSX_ILL: str = std::string("SHY_ABSX_ILL"); return out << str;
        case CPU::SHX_ABSY_ILL: str = std::string("SHX_ABSY_ILL"); return out << str;
        case CPU::TAS_ABSY_ILL: str = std::string("TAS_ABSY_ILL"); return out << str;
        case CPU::LXA_IMM_ILL: str = std::string("LXA_IMM_ILL"); return out << str;
        case CPU::LAS_ABSY_ILL: str = std::string("LAS_ABSY_ILL"); return out << str;
        case CPU::SBX_IMM_ILL: str = std::string("SBX_IMM_ILL"); return out << str;
        case CPU::USBC_IMM_ILL: str = std::string("USBC_IMM_ILL"); return out << str;
        default: str = std::string("UNKNOWN"); return out << str; // unreachable 
    }
};


