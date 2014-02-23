/**
 * @file debug.cpp
 * 
 * @author Joseph Cox
 * 
 * @brief This file holds debugging functions such as an inverse assembler and 
 * getters for internal CPU variables.
 *
 * @todo Null                
 *
 * @bugs None.
 */



#include "include.h"



/*
	Any opcode undefined is treated as a don't care.  The return value
	of this may be anything.
*/

/** 
 * @param unsigned char opcode The opcode to decode to mnemonics.
 * @param char *mnemonic A pointer to some space to put the mnemonic.
 */

void debug::get_mnemonic(unsigned char opcode, char *mnemonic)
{
	unsigned char highnib;
	unsigned char lownib;
	
	const char* const zero[] = {"BRK", "BPL", "JSR", "BMI", "RTI", "BVC",  //When lownib == 0
		       "RTS", "BVS", "NULL", "BCC", "LDY", "BCS", 	  
		       "CPY", "BNE", "CPX", "BEQ"};

	const char* const one_or_five_or_nine_or_D[] = {"ORA", "AND", "EOR", "ADC",  //The code repeats every other	 
 					   "STA", "LDA", "CMP", "SBC"}; //for the high nibble.  This
				     			     		//means that we do a floor divide
							     		//by 2 for the op code.
	const char* const four_or_C[] = {"NULL", "BIT", "JMP", "JMP", "STY", "LDY", "CPY", "CPX"};

	const char* const six_or_A_or_E[] = {"ASL", "ROL", "LSR", "ROR", "STX", "LDX", "DEC", "INC"};

	const char* const A_highbit[] = {"TXA", "TXS", "TAX", "TSX", "DEX", "NULL", "NOP", "NULL"};

	const char* const eight[] = {"PHP", "CLC", "PLP", "SEC", "PHA", "CLI", "PLA", "SEI",
			"DEY", "TYA", "TAY", "CLV", "INY", "CLD", "INX", "SED"};

	highnib = (opcode >> 4) & 0x0F;
	lownib = opcode & 0x0F;
	switch(lownib){
	case 0x0:
		strcpy(mnemonic, zero[highnib]); 
		break;
	case 0x1:
	case 0x5:
	case 0x9:
	case 0xD:
		strcpy(mnemonic, one_or_five_or_nine_or_D[highnib/2]);
		break;
	case 0x2:
		strcpy(mnemonic, "LDX");
		break;
	case 0x4:
	case 0xC:	
		strcpy(mnemonic, four_or_C[highnib/2]);
		break;	
	case 0x6:
	case 0xE:
		strcpy(mnemonic, six_or_A_or_E[highnib/2]);
		break;
	case 0xA:
		if(highnib > 7)
			strcpy(mnemonic, A_highbit[highnib - 8]);
		else
			strcpy(mnemonic, six_or_A_or_E[highnib / 2]);
		break;
	case 0x8:
		strcpy(mnemonic, eight[highnib]);
		break;
	default:
		strcpy(mnemonic, "NULL");
	}
}

/** 
 * @remarks This keeps track of the bytes around the program counter.
 */

void debug::debug_instruction(void)
{
	original_address = registers.pc;
	run_instruction();

	if(opsize == 2)
		original_address = mem_read(original_address + 1);
	else if(opsize == 3)
		original_address = (mem_read(original_address  + 1)) | (mem_read(original_address + 2) << 8);
	else
		original_address = 0;
	cycles_total += get_cycles();
}

/**
 * @description This prints out the address pointed to by the instruction.
 * @param FILE *out This is the output file.
 */

void debug::print_address(char *out)
{
	char *address = new char[20];

	if(opsize == 2)
		sprintf(address, "$%02x", original_address);
	else if(opsize == 3)
		sprintf(address, "$%02x%02x", (original_address & 0xFF00) >> 8, original_address & 0xFF);
	
	switch(get_addr_mode()){
	case ACUM:
		sprintf(out, "A");
		break;
	case ABS:
	case REL:
	case ZPG:
		sprintf(out, "%s", address);
		break;
	case ABSX:
	case ZPGX:
		sprintf(out, "%s,X", address);
		break;
	case ABSY:
	case ZPGY:
		sprintf(out, "%s,Y", address);
		break;
	case IMED:
		sprintf(out, "#%s", address);
		break;
	case IMPL:
		break;
	case IND:
		sprintf(out, "#%s", address);
		break;
	case XIND:
		sprintf(out, "(%s,X)", address);
		break;
	case INDY:
		sprintf(out, "(%s),Y", address);
		break;
	}
	delete []address;
}


/** @return int The gotten opcode.
 */

int debug::get_opcode(void)
{
	return opcode;
}

/** @param int register_type This is the register value to return.
 *  @return unsigned char The value of the register.
 */

unsigned char debug::get_registers(int register_type)
{
	switch(register_type){
	case RA:
		return registers.areg;
	case RX:
		return registers.xreg;
	case RY:
		return registers.yreg;
	case RS:
		return registers.stackp;
	default:
		return 0;
	}
}

/** @return unsigned short The current value of the pc.*/

unsigned short debug::get_pc(void)
{
	return registers.pc;
}

/** @param unsigned short address The address to read from.
 *  @return unsigned char The byte read.
 */

unsigned char debug::get_mem(unsigned short address)
{
	return mem_read(address);
}

/** 
 * @return int The size of the current opcode.
 */

int debug::get_opsize(void)
{
	return opsize;
}
/** @param int flag_type The flag to return
 *  @return int The flag's value
 */

int debug::get_flag(int flag_type)
{
	switch(flag_type){
	case C:
		return registers.fc;
	case Z:
		return registers.fz;
	case I:
		return registers.fi;
	case D:
		return registers.fd;
	case B:
		return registers.fb;
	case V:
		return registers.fv;
	case N:
		return registers.fn;
	default:
		return 0;
	}
}

void debug::print_io_buffer(void)
{
        char buf[0x200];
        get_buffer(buf);
	printf("Here is the current buffer: %s\n", buf);
	
}

int debug::get_total_cycles(void)
{
        return cycles_total;
}


int debug::map_label(char *label)
{
        struct subword * word = get_word(label);
	
	
	if(word == NULL || word->type != LABEL_T)
	        return -1;
	
	else
	        return strtohex(word->value);
}

int debug::get_prog_size(void)
{
        return get_program_size();
}
