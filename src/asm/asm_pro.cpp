namespace assembler_constants
{
	/* 
	   An enumeration of the instruction set's index w.r.t.
	   asm_pro::iset.
	   Note that setcard is the cardinality of the instruction set.
	*/

	enum i_set_element{
		ADC, AND, ASL, BCC, BCS, BEQ,
		BIT, BMI, BNE, BPL, BRK, BVC,
		BVS, CLC, CLD, CLI, CLV, CMP,
		CPX, CPY, DEC, DEX, DEY, EOR,
		INC, INX, INY, JMP, JSR, LDA,
		LDX, LDY, LSR, NOP, ORA, PHA,
		PHP, PLA, PLP, ROL, ROR, RTI,
		RTS, SBC, SEC, SED, SEI, STA,
		STX, STY, TAX, TAY, TSX, TXA,
		TXS, TYA, 
		
		setcard
	};

	//IMPL is not used in calculations, so addrcard skips it.
	enum addr_mode{
		IMED, ZPG,  ZPGX, ZPGY, ABS,  ABSX,
		ABSY, INDY, XIND, IND,  ACUM, addrcard,
		IMPL
	};
	
	/*This indexes to the assembler directive strings.*/
	enum dir_index{
		ALIGN, BEGIN,   BYTE,  BYTE1, END,  ENDMAC,
		EQU,   INCLUDE, MACRO, ORG,   TEXT, WORD, 
		dircard
	};


}


using namespace assembler_constants;

static int scmp(const void *a, const void *b);

const char * const asm_pro::iset[] = { "ADC" ,"AND", "ASL", "BCC", "BCS", "BEQ",
					"BIT", "BMI", "BNE", "BPL", "BRK", "BVC",
					"BVS", "CLC", "CLD", "CLI", "CLV", "CMP",
					"CPX", "CPY", "DEC", "DEX", "DEY", "EOR", 
					"INC", "INX", "INY", "JMP", "JSR", "LDA", 
					"LDX", "LDY", "LSR", "NOP", "ORA", "PHA",
					"PHP", "PLA", "PLP", "ROL", "ROR", "RTI", 
					"RTS", "SBC", "SEC", "SED", "SEI", "STA",
					"STX", "STY", "TAX", "TAY", "TSX", "TXA",
					"TXS", "TYA"};
					
const char * const asm_pro::directives[] = { ".ALIGN", ".BEGIN",  ".BY",   ".BYTE", 
					      ".END",   ".ENDMAC", ".EQU",  ".INCLUDE", 
					      ".MACRO", ".ORG",    ".TEXT", ".WORD"};

/*This gives the allowed address modes for the corresponding instruction (By alphabet).
  It is in one-hot form, with the LSB corresponding to the first entry
  in the addr_mode enum and going up from there. Note that relative addresses
  are treated as zeropages.
  For example, with ADC, we have the binary code 1 1111 0111.  This corresponds to
  the following addressing modes: IMED, ZPG, ZPGX, ABS, ABSX, ABSY, INDY, and
  XIND.*/  	

const int asm_pro::isetmodes[] = {0x01F7, 0x01F7, 0x0436, 0x0002, 0x0002, 0x0002,
				   0x0012, 0x0002, 0x0002, 0x0002, 0x1000, 0x0002,
				   0x0002, 0x1000, 0x1000, 0x1000, 0x1000, 0x01F7,
				   0x0013, 0x0013, 0x0036, 0x1000, 0x1000, 0x01F7,
				   0x0036, 0x1000, 0x1000, 0x0210, 0x0010, 0x01F7,
				   0x005B, 0x0037, 0x0436, 0x1000, 0x01F7, 0x1000,
				   0x1000, 0x1000, 0x1000, 0x0436, 0x0436, 0x1000,
				   0x1000, 0x01F7, 0x1000, 0x1000, 0x1000, 0x01F6,
				   0x001A, 0x0016, 0x1000, 0x1000, 0x1000, 0x1000,
				   0x1000, 0x1000}; 


//These are what the allowed addressing modes look like.  When checking for valid modes,
//all numbers are converted to generic X's and compared with these strings.  For example,
//if we have the address $3234, when it is tested it will have become $XXXX.  However,
//if we have $323, it will have another X appended, becoming $XXXX instead of $XXX.

const char * const asm_pro::addr_modes[] = { "#$XX",     "$XX",     "$XX,X",  "$XX,Y", 
					      "$XXXX",    "$XXXX,X", "$XXXX,Y","($XX),Y",
					      "($XX,X)",  "($XXXX)", "A"};


/*This maps asm_pro::addr_modes to each mode's actual name.*/
const char * const asm_pro::addr_mode_names[] = { "IMED", "ZPG", "ZPGX", "ZPGY", 
						   "ABS", "ABSX", "ABSY", "INDY", 
						   "XIND", "IND", "ACUM", "", 
						   "IMPL"};
unsigned char asm_6502::opcode_map(int opcode_index, int addr_mode)
{
        unsigned char opcode = 0;
	/*This switch statement takes care of the instructions which are one
	  to one or one to two with the opcodes.*/

        switch(opcode_index){
	case BCC: return 0x90;
	case BCS: return 0xB0;
	case BEQ: return 0xF0;
	case BIT:
	        if(addr_mode == ZPG)
		        return 0x24;
		else
		        return 0x2C;
	case BMI: return 0x30;
	case BNE: return 0xD0;
	case BPL: return 0x10;
	case BRK: return 0x00;
	case BVC: return 0x50;
	case BVS: return 0x70;
	case CLC: return 0x18;
	case CLD: return 0xDA;		
	case CLI: return 0x58;
	case CLV: return 0xB8;
	case DEX: return 0xCA;
	case DEY: return 0x88;
	case INX: return 0xE8;
	case INY: return 0xC8;
	case JMP:
	        if(addr_mode == IND)
		        return 0x6C;
		else
		        return 0x4C;
	case JSR: return 0x20;
	case NOP: return 0xEA;
	case PHA: return 0x48;
	case PHP: return 0x08;
	case PLA: return 0x68;
	case PLP: return 0x28;
	case RTI: return 0x40;
	case RTS: return 0x60;
	case SEC: return 0x38;
	case SED: return 0xF8;
	case SEI: return 0x78;
	case TAX: return 0xAA;
	case TAY: return 0xA8;
	case TXA: return 0x8A;
	case TXS: return 0x9A;
	case TYA: return 0x98;
	case TSX: return 0xBA;
	default:
	        break;
	}
	
	if(opcode == 0){
	        //This sets the first three bits of an opcode.
	        switch(opcode_index){						
		case ORA:
		case ASL: opcode = 0x0; break;		  
		case AND:
		case ROL: opcode = 0x20; break;		  
		case EOR:
		case LSR: opcode = 0x40; break;
		case ADC:
		case ROR: opcode = 0x60; break;
		case STA:
		case STX:
		case STY: opcode = 0x80; break;
		case LDA:
		case LDX:
		case LDY: opcode = 0xA0; break;
		case CMP:
		case DEC:
		case CPY: opcode = 0xC0; break;
		case SBC:
		case INC:
		case CPX: opcode = 0xE0; break;
		default: break;
		}
		//Next three bits!
		switch(addr_mode){
		case XIND:
		  opcode |= 0x00; break;
		case ZPG: opcode |= 0x04; break;
		case IMED:
		case ACUM: opcode |= 0x08;	break;
		case ABS: opcode |= 0x0C; break;
		case INDY: opcode |= 0x10; break;
		case ZPGX:
		case ZPGY: opcode |= 0x14; break;
		case ABSY: opcode |= 0x18; break;
		case ABSX: opcode |= 0x1C; break;
		default:
		        break;
		}
		//Get final two bits and check for exceptions
		//to addressing rules.
		switch(opcode_index){
		case ORA:    case AND:    case EOR:    case ADC:
		case STA:    case LDA:	  case CMP:    case SBC:
		  opcode |= 0x01; break;
		case ASL:    case ROL:    case LSR:    case ROR:
		case STX:    case LDX:    case DEC:    case INC:
		        if(addr_mode == IMED){
			        opcode &= ~(0x1C);
				opcode |= 0x00;
			}
			opcode |= 0x02;
			break;
		case STY:    case LDY:    case CPY:    case CPX:
		        if(addr_mode == IMED){
			        opcode &= ~(0x1C);
				opcode |= 0x00;
			}
			opcode |= 0x00;
			break;
		default:
		        break;
		  
		}
	}
	return opcode;
}

