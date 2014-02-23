/**
 * @file jpc.cpp
 * 
 * @author Joseph Cox
 *
 * @brief This file holds keystone functions, allowing the integration
 * of the emulator's components.
 *                             
 * @todo Null                
 *
 * @bugs None.
 */




#include "include.h"

/*Memory mapping for the emulated device:
  0 - 0x1FFF: Ram
  0x2000 - 0x3FFF: IO mirrored every two bytes
  0x4000 - 0x7FFF: Exit the emulator
  0x8000 - 0xFFFF: Program Memory
*/

/**
 * @description Read a byte from memory.
 * @param unsigned short address This is the address to read from
 * @return unsigned char The byte from memory.
 */

unsigned char jpc::mem_read(unsigned short address)
{


	if(address < 0x2000)
		return get_ram(address);
	else if(address < 0x4000)
		return gpinreg();
	if(address < 0x8000){
		return 0;
	}
	else
		return get_rom(address - 0x8000);
#ifdef DEBUG
	cycles_total += cycles;	
#endif //DEBUG
	cycles = 0; 
}

/**
 * @description Write a byte to memory.
 * @param unsigned short address This is the address to write to.
 * @param char value This is the byte to write to memory.
 */

void jpc::mem_write(unsigned short address, char value)
{
	if(address < 0x2000)
		set_ram(address, value);
	else if(address < 0x4000)
		writedecoder(address, value);
	else if(address < 0x8000){
		exit(0);
	}		
	else{
		printf("Error: Cannot write to ROM\n");
		exit(1);
	}
	cycles_total += cycles;
	cycles = 0;
}
