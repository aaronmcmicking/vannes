#pragma once

#include <iostream>
#include "../CPU.hpp"
#include "../../common/nes_assert.hpp"


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
            ASL();
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
            LSR();
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
            ROL();
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
            ROR();
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
            std::cout << "ERROR: Got unreachable instruction! How is this possible??? Bad instruction was " << instruction << std::endl;
            assert(0 && "unreachable");
            break;
    }
} // execute_instruction


        void LDA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
     
        void LDX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void LDY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void STA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void STX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 	
        void STY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
     

        /* Register Transfers */
        void TAX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 
        void TAY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 
        void TXA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 
        void TYA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}
 

        /* Stack Operations */
        void TSX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void TXS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void PHA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void PHP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void PLA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void PLP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Logical */
        void AND(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void EOR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ORA(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BIT(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Arithmetic */
        void ADC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SBC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CMP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CPY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Increments & Decrements */
        void INC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void INX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void INY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void DEC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void DEX(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void DEY(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Shifts */
        void ASL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void LSR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ROL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ROR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Jumps & Calls */
        void JMP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void JSR(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void RTS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Branches */
        void BCC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BCS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BEQ(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BMI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BNE(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BPL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BVC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void BVS(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Status Flag Changes */
        void CLC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CLD(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CLI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void CLV(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SEC(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SED(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SEI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* System Functions */
        void BRK(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void NOP(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void RTI(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}


        /* Unofficial/Illegal opcodes */
        void NOP_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void JAM_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SLO_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void RLA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SRE_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void RRA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SAX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void LAX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void DCP_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ISC_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ANC_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ALR_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ARR_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void ANE_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SHA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SHY_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SHX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void TAS_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void LXA_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void LAS_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void SBX_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

        void USBC_ILL(){
			VNES_ASSERT(0 && "UNIMPLEMENTED INSTRUCTION");
		}

