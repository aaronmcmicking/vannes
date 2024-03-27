#pragma once

#include <stdint.h>
#include "imp/RAM.cpp"
#include "../common/typedefs.hpp"

// The NES CPU is a modified version of the MOS 6502 called the Ricoh 2A03.
// It removes some instructions from the 6502 and includes and APU in the CPU.
class CPU{
    public:
        // see https://www.masswerk.at/6502/6502_instruction_set.html#ADC
        enum CPU_ADDRESSING_MODE{
            ACC = 1,    // Accumulator 
            ABS,        // Absolute
            ABSX,       // Absolute, X-indexed
            ABSY,       // Absolute, Y-indexed
            IMM,        // Immediate
            IMPL,       // Implied
            IND,        // Indirect
            XIND,       // X-indexed, indirect
            INDY,       // Indirect, Y-indexed
            REL,        // Relative
            ZPG,        // Zeropage
            ZPGX,       // Zeropage, X-indexed
            ZPGY        // Zeropage, Y-indexed
        };

        enum CPU_INSTR : uint8_t{
            /* FORMAT:
             * OPCODE_ADDRMODE = HEX // OPERATION, UPDATED FLAGS
             */
            /* Load/Store */
            LDA_XIND = 0xA1,    // Load Accumulator 	N,Z
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
            STA_XIND = 0x81, 	// Store Accumulator 	 
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
            AND_XIND = 0x21, 	    // Logical AND 	            N,Z
            AND_ZPG  = 0x25, 	    
            AND_IMM  = 0x29, 	    
            AND_ABS  = 0x2D, 	    
            AND_INDY = 0x31, 	    
            AND_ZPGX = 0x35, 	    
            AND_ABSY = 0x39, 	    
            AND_ABSX = 0x3D, 	    
            EOR_XIND = 0x41, 	    // Exclusive OR 	        N,Z
            EOR_ZPG  = 0x45, 	    
            EOR_IMM  = 0x49, 	    
            EOR_ABS  = 0x4D, 	    
            EOR_INDY = 0x51, 	    
            EOR_ZPGX = 0x55, 	    
            EOR_ABSY = 0x59, 	    
            EOR_ABSX = 0x5D, 	    
            ORA_XIND = 0x01, 	    // Logical Inclusive OR 	N,Z
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
            ADC_XIND = 0x61, 	    // Add with Carry 	    N,V,Z,C 
            ADC_ZPG  = 0x65, 	    
            ADC_IMM  = 0x69, 	    
            ADC_ABS  = 0x6D, 	    
            ADC_INDY = 0x71, 	    
            ADC_ZPGX = 0x75, 	    
            ADC_ABSY = 0x79, 	    
            ADC_ABSX = 0x7D, 	    
            SBC_XIND = 0xE1, 	    // Subtract with Carry 	N,V,Z,C
            SBC_ZPG  = 0xE5, 	    
            SBC_IMM  = 0xE9, 	    
            SBC_ABS  = 0xED, 	    
            SBC_INDY = 0xF1, 	    
            SBC_ZPGX = 0xF5, 	    
            SBC_ABSY = 0xF9, 	    
            SBC_ABSX = 0xFD, 	    
            CMP_XIND = 0xC1, 	    // Compare accumulator 	N,Z,C
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
            SLO_XIND_ILL = 0x03,
            SLO_INDY_ILL = 0x13,
            SLO_ZPG_ILL  = 0x07,
            SLO_ZPGX_ILL = 0x17,
            SLO_ABSY_ILL = 0x1B,
            SLO_ABS_ILL  = 0x0F,
            SLO_ABSX_ILL = 0x1F,

            /* RLA = AND combined with ROL */
            RLA_XIND_ILL = 0x23,
            RLA_INDY_ILL = 0x33,
            RLA_ZPG_ILL  = 0x27,
            RLA_ZPGX_ILL = 0x37,
            RLA_ABSY_ILL = 0x3B,
            RLA_ABS_ILL  = 0x2F,
            RLA_ABSX_ILL = 0x3F,

            /* SRE = LSR combined with EOR */
            SRE_XIND_ILL = 0x43,
            SRE_INDY_ILL = 0x53,
            SRE_ZPG_ILL  = 0x47,
            SRE_ZPGX_ILL = 0x57,
            SRE_ABSY_ILL = 0x5B,
            SRE_ABS_ILL  = 0x4F,
            SRE_ABSX_ILL = 0x5F,

            /* RRA = ROR combined with ADC */
            RRA_XIND_ILL = 0x63,
            RRA_INDY_ILL = 0x73,
            RRA_ZPG_ILL  = 0x67,
            RRA_ZPGX_ILL = 0x77,
            RRA_ABSY_ILL = 0x7B,
            RRA_ABS_ILL  = 0x6F,
            RRA_ABSX_ILL = 0x7F,

            /* SAX */
            SAX_XIND_ILL = 0x83,
            SAX_ZPG_ILL = 0x87,
            SAX_ZPGY_ILL = 0x97,
            SAX_ABS_ILL = 0x8F,

            /* LAX = LDA combined with LDX */
            LAX_XIND_ILL = 0xA3,
            LAX_INDY_ILL = 0xB3,
            LAX_ZPG_ILL  = 0xA7,
            LAX_ZPGY_ILL = 0xB7,
            LAX_ABS_ILL  = 0xAF,
            LAX_ABSY_ILL = 0xBF,

            /* DCP = LDA combined with TSX */
            DCP_XIND_ILL = 0xC3,
            DCP_INDY_ILL = 0xD3,
            DCP_ZPG_ILL  = 0xC7,
            DCP_ZPGX_ILL = 0xD7,
            DCP_ABSY_ILL = 0xDB,
            DCP_ABS_ILL  = 0xCF,
            DCP_ABSX_ILL = 0xDF,

            /* ISC = INC combined with SBC */
            ISC_XIND_ILL = 0xE3,
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

    public:
        CPU(RAM& _ram);

    private:
        RAM& ram;

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
        bit carry;
        bit zero;
        bit interrupt_disable;
        bit b_flag;
        bit decimal;
        bit overflow;
        bit negative;
        uint8_t status_as_int(); // returns the packed status register

        uint64_t cycles;

        uint8_t fetch_instruction();
        void    execute_instruction(uint8_t instruction);

        enum CPU_ADDRESSING_MODE addr_mode;
        void        set_addr_mode(enum CPU_ADDRESSING_MODE mode);
        uint16_t    fetch_address();

        uint8_t read_mem(uint16_t addr);
        void    write_mem(uint16_t addr, uint8_t data);
        void    push_stack(uint8_t data);
        uint8_t pop_stack();

        // see https://web.archive.org/web/20200129081101/http://users.telenet.be:80/kim1-6502/6502/proman.html
        // for power-up sequence details
        void power_up();
        void engage_reset();
        void release_reset();

        void        write_nmi_vec(uint16_t data);
        uint16_t    read_nmi_vec();
        void        write_reset_vec(uint16_t data);
        uint16_t    read_reset_vec();
        void        write_brk_vec(uint16_t data);
        uint16_t    read_brk_vec();

        void raise_interrupt(bool maskable);
        void return_from_interrupt();

        /* INSTRUCTIONS */
        /* Load/Store */
        void LDA();     // Load Accumulator 	N,Z
        void LDX(); 	// Load X Register 	    N,Z
        void LDY(); 	// Load Y Register 	    N,Z
        void STA(); 	// Store Accumulator 	 
        void STX(); 	// Store X Register 	 
        void STY();     // Store Y Register 	 

        /* Register Transfers */
        void TAX(); 	// Transfer accumulator to X 	N,Z
        void TAY(); 	// Transfer accumulator to Y 	N,Z
        void TXA(); 	// Transfer X to accumulator 	N,Z
        void TYA(); 	// Transfer Y to accumulator 	N,Z

        /* Stack Operations */
        void TSX(); 	// Transfer stack pointer to X 	    N,Z
        void TXS(); 	// Transfer X to stack pointer 	 
        void PHA(); 	// Push accumulator on stack 	 
        void PHP(); 	// Push processor status on stack 	 
        void PLA(); 	// Pull accumulator from stack 	    N,Z
        void PLP(); 	// Pull processor status from stack All

        /* Logical */
        void AND(); 	// Logical AND 	            N,Z
        void EOR(); 	// Exclusive OR 	        N,Z
        void ORA(); 	// Logical Inclusive OR 	N,Z
        void BIT(); 	// Bit Test 	            N,V,Z

        /* Arithmetic */
        void ADC(); 	// Add with Carry 	    N,V,Z,C 
        void SBC(); 	// Subtract with Carry 	N,V,Z,C
        void CMP(); 	// Compare accumulator 	N,Z,C
        void CPX(); 	// Compare X register 	N,Z,C
        void CPY(); 	// Compare Y register 	N,Z,C

        /* Increments & Decrements */
        void INC(); 	// Increment a memory location 	N,Z
        void INX(); 	// Increment the X register 	N,Z
        void INY(); 	// Increment the Y register 	N,Z
        void DEC(); 	// Decrement a memory location 	N,Z
        void DEX(); 	// Decrement the X register 	N,Z
        void DEY(); 	// Decrement the Y register 	N,Z

        /* Shifts */
        void ASL(); 	    // Arithmetic Shift Left 	N,Z,C
        void ASL_eACC(); 	// ASL in Accumulator mode 	N,Z,C
        void LSR(); 	 // Logical Shift Right 	    N,Z,C
        void LSR_eACC(); // LSR in Accumulator mode N,Z,C
        void ROL(); 	 // Rotate Left 	            N,Z,C
        void ROL_eACC(); // ROL in Accumulator mode N,Z,C
        void ROR(); 	 // Rotate Right 	        N,Z,C
        void ROR_eACC(); // ROR in Accumulator mode N,Z,C

        /* Jumps & Calls */
        void JMP();     // Jump to another location 	 
        void JSR(); 	// Jump to a subroutine 	 
        void RTS(); 	// Return from subroutine 	 

        /* Branches */
        void BCC(); 	// Branch if carry flag clear 	 
        void BCS(); 	// Branch if carry flag set 	 
        void BEQ(); 	// Branch if zero flag set 	 
        void BMI(); 	// Branch if negative flag set 	 
        void BNE(); 	// Branch if zero flag clear 	 
        void BPL(); 	// Branch if negative flag clear 	 
        void BVC(); 	// Branch if overflow flag clear 	 
        void BVS(); 	// Branch if overflow flag set 	 

        /* Status Flag Changes */
        void CLC(); 	// Clear carry flag 	            C
        void CLD(); 	// Clear decimal mode flag 	        D
        void CLI(); 	// Clear interrupt disable flag 	I
        void CLV(); 	// Clear overflow flag 	            V
        void SEC(); 	// Set carry flag 	                C
        void SED(); 	// Set decimal mode flag 	        D
        void SEI(); 	// Set interrupt disable flag 	    I

        /* System Functions */
        void BRK(); 	// Force an interrupt 	    B
        void NOP(); 	// No Operation (illegal nop's also exist)
        void RTI();	    // Return from Interrupt 	All

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
};

