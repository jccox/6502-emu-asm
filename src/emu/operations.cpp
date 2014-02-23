/** @author Joseph Cox
 *  @date 7/13/2013
 *  @brief This file contains definitions of functions which emulate
 *	   operations in the 6502 microprocessor.
 *
 *  @remarks Many of the functions here use a multiplexing variable
 *   	     in order to allow a single function to be abstracted
 *           to handle multiple registers at once.
**/	

#include "include.h"

void cpu6502::adcf(void)
{
	int total;

	total = registers.areg + mem_read(address_value) + registers.fc;

	registers.fv = (getbit(total, 7) != getbit(registers.areg, 7)? 1:0);
	registers.fn = (getbit(total,7)? 1:0);
	registers.fz = (total % 256 == 0? 1:0);
	registers.fc = total > 256? 1:0;
	registers.areg = total;

	++cycles;

}



void cpu6502::andf(void)
{
	registers.areg &= mem_read(address_value);
	registers.fn = (getbit(registers.areg,7)? 1:0);
	registers.fz = (registers.areg == 0? 1:0);

	++cycles;
}

void cpu6502::bit(void)
{
	unsigned char test;
	
	test = registers.areg & mem_read(address_value);
	registers.fn = getbit(test, 7);
	registers.fv = getbit(test, 6);
	registers.fz = ((test == 0)? 1:0);
	
	++cycles;
}

void cpu6502::branch(int type)
{
	char val;

	++cycles;
	switch(type){
	case 0x1:
		val = !registers.fn;
		break;
	       
	case 0x3:
		val = registers.fn;
		break;
	case 0x5:
		val = !registers.fv;
		break;
	case 0x7:
		val = registers.fv;
		break;
	case 0x9:
		val = !registers.fc;
		break;	  
	case 0xb:
		val = registers.fc;
		break;
	case 0xd:
		val = !registers.fz;
		break;
	case 0xf:
		val = registers.fz;
		break;
	default:
		return;
	}

	++cycles;
	if(val == 0){
		return;
	}
	
	registers.pc += (char) address_value;
	++cycles;
	if(((registers.pc - (char) address_value) & 0xFF00) != (registers.pc & 0xFF00))
		++cycles;

}



void cpu6502::brk(void)
{
	stackpush(((registers.pc >> 8) & 0xFF));
	++cycles;
	stackpush((registers.pc & 0xFF));	
	++cycles;
	pack_flags();
	stackpush(registers.flags | 0x10); //Push on flags with BRK set.
	++cycles;
	registers.pc = mem_read(0xFFFE);
	++cycles;
	registers.pc |= mem_read(0xFFFF) << 8;
	++cycles;
}

void cpu6502::cmpf(int reg)
{
	char value = mem_read(address_value);
	char result;
	switch(reg){
	case RA:
		result = registers.areg - value;
		break;
	case RX:
		result = registers.xreg - value;
		break;
	case RY:
		result = registers.yreg - value;
		break;
	default:
		result = 0;
		break;
	}
	registers.fn = getbit(result, 7);
	registers.fc = ((result + value) >= value)? 1:0;
	registers.fz = ((result == 0)? 1:0);

	++cycles;
}


	
void cpu6502::eorf(void)
{
	registers.areg ^= mem_read(address_value);
	registers.fn = getbit(registers.areg, 7);
	registers.fz = ((registers.areg == 0)? 1:0);
	++cycles;
}

void cpu6502::incdecf(int type)
{

	fetched_data = mem_read(address_value);
	++cycles;
	mem_write(address_value, fetched_data);
	++cycles;
	if(type == INC_T)
		++fetched_data;
	else
		--fetched_data;
	registers.fn = getbit(fetched_data, 7);
	registers.fz = ((fetched_data == 0)? 1:0);	
	mem_write(address_value, fetched_data);
	++cycles;
}

void cpu6502::jmp(void)
{
	registers.pc = address_value;
}

void cpu6502::jsr(void)
{
	
	--registers.pc;
	++cycles;
	stackpush(registers.pc >> 8);
	++cycles;
	stackpush(registers.pc & 0xFF);
	++cycles;
	registers.pc = address_value;
}
void cpu6502::oraf(void)
{
	registers.areg |= mem_read(address_value);
	registers.fn = getbit(registers.areg, 7);
	registers.fz = ((registers.areg == 0)? 1:0);

	++cycles;
}

void cpu6502::rti(void)
{
	int byte;
	

	++registers.stackp;
	++cycles;
	stackpeek(registers.flags);
	unpack_flags();
	++registers.stackp;
	++cycles;
	stackpeek(registers.pc);
	++registers.stackp;
	++cycles;
	stackpeek(byte);
	registers.pc |= ((byte << 8) & 0xFF00);
	++cycles;
}

void cpu6502::rts(void)
{
	int byte;

	++registers.stackp;
	++cycles;
	stackpeek(registers.pc);
	++registers.stackp;
	++cycles;
	stackpeek(byte);
	registers.pc |= ((byte << 8) & 0xFF00);
	++cycles;
	++registers.pc;
	++cycles;
}
void cpu6502::sbcf()
{
	int total;
	
	total = registers.areg - (mem_read(address_value)) - (registers.fc ^ 1);
	registers.fv = (total > 127 || total < -128)? 1:0;

	registers.fc = (total >= 0)? 1:0;
	registers.fn = getbit(total, 7);
	registers.fz = (total == 0)? 1:0;
	registers.areg = total;

	++cycles;
}

void cpu6502::shiftf(int dir)
{
	char temp;
	if(addr_mode == ACUM)
		fetched_data = registers.areg;
	else{
		fetched_data = mem_read(address_value);
		++cycles;
		mem_write(address_value, fetched_data);
		++cycles;
	}
	
	if(dir == RLEFT || dir == RRIGHT){
		if(dir == RLEFT){
			temp = getbit(fetched_data, 7);
			fetched_data <<= 1;
			fetched_data &= 0xFE;
			fetched_data |= registers.fc;
		}
		else{
			temp = getbit(fetched_data, 0);
			fetched_data >>= 1;
			fetched_data &= 0x7F;
			fetched_data |= ((registers.fc == 0)? 0x0:0x80);
		}
		registers.fc = temp;
		registers.fz = (fetched_data == 0)? 1:0;
		registers.fn = getbit(fetched_data, 7);
	}
	else{
		if(dir == LRIGHT){
			registers.fn = 0;
			registers.fc = (getbit(fetched_data,0)? 1:0);
			fetched_data >>= 1;
			fetched_data &= 0x7F;
		}
		else{
			registers.fc = (getbit(fetched_data,7)? 1:0);
			fetched_data <<= 1;
			fetched_data &= 0xFE;
			registers.fn = (getbit(fetched_data,7)? 1:0);
		}		
		registers.fz = (fetched_data == 0? 1:0);
	}
	if(addr_mode == ACUM){
		registers.areg = fetched_data;
	}
	else{
		mem_write(address_value, fetched_data);
		++cycles;
	}
}

void cpu6502::one_byte(void)
{
	char highnibble = opcode >> 4;
	char result;
	char flagtrue;
	
	/***Stack ops***/
	if(((highnibble) < 0x8) && ((highnibble) % 2 == 0)){
		//Cycle # 2 is wasted with stacks.
		++cycles;
		switch(opcode){
		case PHP:
			pack_flags();
			stackpush(registers.flags);
			break;
		case PLP:

			++registers.stackp;
			++cycles;
			pack_flags();
			stackpeek(registers.flags);
			break;
		case PHA:
			stackpush(registers.areg);
			break;
		case PLA:

			++registers.stackp;
			++cycles;
			stackpeek(registers.areg);
			registers.fn = getbit(registers.areg, 7);
			registers.fz = ((registers.xreg == 0)? 1:0);
			break;
		default:
			break;
		}
	}

	/***Flag ops***/
	else if(((highnibble) & 0x1) == 0x1 && (highnibble) != 0x9){
		flagtrue = ((highnibble / 2) & 0x1);
		switch(highnibble / 4){  //This seperates the opcode into different flags.
		case CARRY:
			registers.fc = flagtrue;
			break;
		case INT:
			registers.fi = flagtrue;
			break;
		case OVERFLOW:
			registers.fi = 0;
			break;
		case DECI:
			registers.fd = flagtrue;
			break;
		default:
			break;
		}
	}

	/***Register ops***/
	else{
		switch(opcode){
		case DEY:
			result = --registers.yreg;
			break;
		case DEX:
			result = --registers.xreg;
			break;
		case INY:
			result = ++registers.yreg;
			break;
		case INX:
			result = ++registers.xreg;
			break;
		case TAY:
			result = registers.yreg = registers.areg;
			break;
		case TYA:
			result = registers.areg = registers.yreg;
			break;
		case TXA:
			result = registers.areg = registers.xreg;
			break;
		case TXS:
			registers.stackp = registers.xreg;
			return;		
		case TAX:
			result = registers.xreg = registers.areg;
			break;
		case TSX:
			result = registers.xreg = registers.stackp;
			break;
		case NOP:
			return;
		default:
			return;
		}
		registers.fn = getbit((result), 7);
		registers.fz = ((result == 0)? 1:0);
	}
}

void cpu6502::load(int reg)
{
	unsigned char result;
	switch(reg){
	case RA:
		result = registers.areg = mem_read(address_value);
		break;
	case RX:
		result = registers.xreg = mem_read(address_value);
		break;
	case RY:
		result = registers.yreg = mem_read(address_value);
		break;
	default:
		return;
	}
	registers.fn = getbit((result), 7);
	registers.fz = ((result == 0)? 1:0);
	++cycles;
}

void cpu6502::store(int reg)
{
	switch(reg){
	case RA:
		mem_write(address_value, registers.areg);
		break;
	case RX:
		mem_write(address_value, registers.xreg);
		break;
	case RY:
		mem_write(address_value, registers.yreg);
		break;
	default:
		break;
	}
	++cycles;
	
}

