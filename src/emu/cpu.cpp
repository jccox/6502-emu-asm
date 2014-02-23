

/**
 * @file cpu.cpp
 * 
 * @author Joseph Cox
 * 
 * @brief This file holds overhead functions for the CPU. 
 *
 * @todo Null                
 *
 * @bugs None.
 */



#include "cpu.h"


cpu6502::cpu6502(void)
{
        addr_mode = 0;
	opsize = 0;
	fetched_data = 0;
	opmode = 0;
	opcode = 0;
	address_value = 0;
	instruction_state = 0;
	cycles_total = 0;
	cycles = 0;
	interrupt_cycle = 0;
	interrupt = 0;
}

/**
 * @description Reset the computer.  Go to the reset vector,
 * turn on interrupts, and clear the carry flag.
 *
 * @param char *buf The buffer to initialize the ram.
**/


void cpu6502::cpureset(void)
{
	registers.pc = (mem_read(0xFFFD) << 8) | mem_read(0xFFFC);
	registers.fi = 1;
	registers.fc = 0;
}

/**
 * @description Find the addressing mode implied
 * by the currently running opcode.
 *
 * @return int The addressing mode of the opcode.
**/

int cpu6502::get_addr_mode(void)
{
	if((opcode & 0x1F) == 0x10)
		return REL;
	else if((opcode & 0x3) == 1){
		switch((opcode & 0x1c) >> 2){
		case 0x0:
			return XIND;	
		case 0x1:
			return ZPG;
		case 0x2:
			return IMED;
		case 0x3:
			return ABS;
		case 0x4:
			return INDY;
		case 0x5:
			return ZPGX;
		case 0x6:
			return ABSY;	
		case 0x7:
			return ABSX;
		default:
			return -1;
		}
	}
	else{
		if((opcode & 0xF) == 8 || ((opcode & 0xF0) > 0x70 && (opcode & 0xF) == 0xA))
			return IMPL;
		switch(opcode){
		case BRK:
		case RTI:
		case RTS:
			return IMPL;
		case JSRA:
			return ABS;
		default: 
			switch((opcode & 0x1c) >> 2){
			case 0x0:
				return IMED;	
			case 0x1:
				return ZPG;
			case 0x2:
				return ACUM;
			case 0x3:
				return ABS;
			case 0x5:
				return ZPGX;
			case 0x7:
				return ABSX;
			default:
				return -1;
			} 
		}
	}
}

/**
 * @description This gets the number of bytes of the 
 * instruction implied by the addressing mode.
 *
 * @return int The size of the opcode based on addressing
 *  mode.
**/

int cpu6502::get_memsize(void)
{
	switch(addr_mode){	
	case ABS:
	case ABSX:
	case ABSY:
		return 3;
	case ACUM:
	case IMPL:
		return 1;
	default:
		return 2;
	}
}
/**
 * @description This gets the number of cycles
 * of an instruction, which is implied by the 
 * addressing mode.
 *
 * @return int The number of cycles consumed while
 *  decoding a given addressing mode.
**/
int cpu6502::get_cycles(void)
{

	switch(addr_mode){
	case ACUM:
	case ZPG:
	case REL:
		return 2;
	case ZPGX:
	case ZPGY:
	case ABS:
		return 3;
	case ABSY:
	case ABSX:
		return 4;
	case IND:
		return 5;
	case XIND:
	case INDY:
		return 6;
	case IMED:
	case IMPL:
	default:
		return 0;
	}
}
/** 
 * @description Decipher whether the
 * instruction is a read, a write, or
 * both.
 *   
 * @return int The type of instruction.
 * @remarks If the instruction is neither
 * of these types, then its type is undefined.
**/

int cpu6502::get_type(void)
{
        if((opcode & 0xF0) == 0x80 || (opcode & 0xF0) == 0x90) 
		return WRITE;
	else if((opcode & 0x3) == 0x1)
		return RW;
	else
		return READ;
}

/** @description This function fetches and reads the program from memory.
 *  For a given instruction, it first reads the opcode, then uses
 *  this value to gather information.  The information includes
 *  the addressing mode of the operation (addr_mode), the total
 *  size of the instruction(opsize), and whether the instruction is
 *  read, write, or both(opmode).
 *
 *  Next, it will read the next bytes of the instruction (There will be
 *  a dummy read if the opsize is 1), and decodes the addressing mode.
**/  

void cpu6502::fetchread(void)
{
        /*First cycle: Fetch the first byte.*/
        opcode = mem_read(registers.pc);
	++registers.pc;
	++cycles;
	
	/*Second cycle: Get the addressing mode
	  and other information.  If it is a one
	  byte instruction, run the instruction
	  on the same cycle.*/
	addr_mode = get_addr_mode();
	opsize = get_memsize();
	opmode = get_type();
	address_value = mem_read(registers.pc);
	++cycles;

	if(opsize > 1)
		++registers.pc;
	if(opsize == 3){
		address_value |= (mem_read(registers.pc) << 8);
		++registers.pc;
		++cycles;
	}

	switch(addr_mode){
	case ZPGX:
		address_value += registers.xreg;
		address_value &= 0xFF;
		break;
	case ZPGY:
		address_value += registers.yreg;
		address_value &= 0xFF;
		break;
	case ABSX:
		address_value += registers.xreg;
		
		//READ type instructions gain a cycle advantage vs other types if no page is crossed.
		if(!(opmode == READ && (address_value & 0xFF00) == ((address_value - registers.xreg) & 0xFF00)))
			++cycles;
		break;
	case ABSY:
		address_value += registers.yreg;
		if(!(opmode == READ && (address_value & 0xFF00) == ((address_value - registers.yreg) & 0xFF00)))
			++cycles;
		break;	

		
	/*For indirect addressing, the 6502 first grabs the byte at
	  address_value.  On the next cycle, we grab the next byte
	  and put it as the high byte of address_value.*/ 
	case IND:
		fetched_data = mem_read(address_value);
		
		//This line dereferences address_value + 1 while keeping the same high byte as before.
		//So, for example, 0x43FF + 1 will equal 0x4300 instead of 0x4400.
		fetched_data += ((mem_read(((address_value + 1) & 0xFF) |  (address_value && 0xFF00))) << 8);
		address_value = fetched_data;
		break;
	case XIND:
		fetched_data = (mem_read(address_value) + registers.xreg);
		fetched_data &= 0xFF;
		
		address_value = mem_read(fetched_data);
		address_value |= (mem_read(((fetched_data + 1) & 0xFF)) << 8);
		break;
	case INDY:
		fetched_data = mem_read(address_value);
		
		address_value = (mem_read(address_value) | (mem_read(address_value + 1) << 8));
		address_value += registers.yreg;
		if((address_value & 0xFF00) == ((address_value - registers.yreg) & 0xFF00))
			++cycles;
		break;
	case IMED:
		address_value = registers.pc - 1;
	default:
		break;
	}
	cycles += get_cycles();
}

/**
 *  @remarks Alot of the secondary switch statements divide
 *  the high nibble by two.  This is because the repetition
 *  in the instruction set allows a nice tradeoff of speed
 * for code size.
**/

void cpu6502::opdecode(void)
{

	unsigned char highnib;
	highnib = (opcode >> 4) & 0x0F;
	
	/*First switch on the low nibble,
	  then switch on the high nibble
	  inside.*/
	switch(opcode & 0x0F){
	case 0x0:
		if((highnib % 2) == 1)
			branch(highnib);
		else{
			switch(highnib){
			case 0x0:
				brk();
				break;
			case 0x2:
				jsr();
				break;
			case 0x4:
				rti();
				break;
			case 0x6:
				rts();
				break;
			case 0x8:
			case 0xA:
				load(RY);
				break;
			case 0xC:
				cmpf(RY);
				break;
			case 0xE:
				cmpf(RX);
				break;
			default:
				break;
			}
		}
		break;
	case 0x1:
	case 0x5:
	case 0x9:
	case 0xD:
		switch(highnib / 2){
		case 0x0:
			oraf();
			break;
		case 0x1:
			andf();
			break;
		case 0x2:
			eorf();
			break;
		case 0x3:
			adcf();
			break;
		case 0x4:
			store(RA);
			break;
		case 0x5:
			load(RA);
			break;
		case 0x6:
			cmpf(RA);
			break;
		case 0x7:
			sbcf();
			break;
		default:
			break;
		}
		break;
	case 0x4:
	case 0xC:	
		switch(highnib / 2){
		case 0x1:
			bit();
			break;
		case 0x2:
		case 0x3:
			jmp();
			break;
		case 0x4:
			store(RY);
			break;
		case 0x5:
			load(RY);
			break;	
		case 0x6:
			cmpf(RY);
			break;
		case 0x7:
			cmpf(RX);
			break;
		default:
			break;
		}
		break;
	case 0x2:
	case 0x6:
	case 0xE:
		switch(highnib / 2){
		case 0x0:
			shiftf(ALEFT);
			break;
		case 0x1:
			shiftf(RLEFT);
			break;
		case 0x2:
			shiftf(LRIGHT);
			break;
		case 0x3:
			shiftf(RRIGHT);
			break;
		case 0x4:
			store(RX);
			break;
		case 0x5:
			load(RX);
			break;
		case 0x6:
			incdecf(DEC_T);
			break;
		case 0x7:
			incdecf(INC_T);
			break;
		default:
			break;
		}
		break;
	case 0xA:
		if(highnib >= 8)
			one_byte();
		else{
			switch(highnib / 2){
			case 0x0:
				shiftf(ALEFT);
				break;
			case 0x1:
				shiftf(RLEFT);
				break;
			case 0x2:
				shiftf(LRIGHT);
				break;
			case 0x3:
				shiftf(RRIGHT);
				break;
			default:
				break;
			}
		}
		break;
	case 0x8:
		one_byte();
		break;
	default:
		break;
	}

}

/** @remarks This function runs a single cycle of the processor.
 *  It works by dividing cycles into three states: fetch, read,
 *  and run.  These are then divided up into individual cycles
 *  where the processor is simulated.
**/
void cpu6502::run_instruction(void)
{
	fetchread();
	opdecode();				
}

void cpu6502::set_interrupt(void)
{
	interrupt |= MI;
}

void cpu6502::set_nmi(void)
{
	interrupt |= NMI;
}
/** @remarks This packs the flags for insertion onto the stack.**/
void cpu6502::pack_flags(void)
{
	registers.flags = 0;

	registers.flags |= registers.fc;
	registers.flags |= registers.fz << 1;
	registers.flags |= registers.fi << 2;
	registers.flags |= registers.fd << 3;
	registers.flags |= registers.fb << 4;
	registers.flags |= registers.fv << 6;	
	registers.flags |= registers.fn << 7;

}
/** @remarks This unpacks the flags after popping off of the stack.**/
void cpu6502::unpack_flags(void)
{
	registers.fc = registers.flags & 0x01;
	registers.fz = registers.flags & 0x02 >> 1;
	registers.fi = registers.flags & 0x04 >> 2;
	registers.fd = registers.flags & 0x08 >> 3;
	registers.fb = registers.flags & 0x10 >> 4;
	registers.fv = registers.flags & 0x40 >> 6;
	registers.fn = registers.flags & 0x80 >> 7;
}


