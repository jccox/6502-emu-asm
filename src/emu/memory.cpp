#include "include.h"



memory32k::memory32k(FILE *in)
{
	rom = new char[0x8000];
	ram = new char[0x2000];
	int i = 0;

	for(i = 0; i < 0x8000; ++i)
 	        rom[i] = 0;

	for(i = 0; i < 0x2000; ++i)
	        ram[i] = 0;
	  
	for(i = 0; (rom[i] = fgetc(in)); ++i);
	
}

memory32k::memory32k(void)
{
        int i = 0;
	
	rom = new char[0x8000];
	ram = new char[0x2000];

	for(i = 0; i < 0x8000; ++i)
 	        rom[i] = 0;

	for(i = 0; i < 0x2000; ++i)
	        ram[i] = 0;

}

memory32k::~memory32k(void)
{
	delete []ram;
	delete []rom;
}

void memory32k::load_rom(char *image)
{
	int i = 0;

	for(i = 0; i < 0x8000; ++i)
		set_rom(i, image[i]);
}

char memory32k::get_ram(unsigned short address)
{
	return ram[address];
}
void memory32k::set_ram(unsigned short address, char value)
{
	ram[address] = value;
}

char memory32k::get_rom(unsigned short address)
{
	return rom[address];
}

void memory32k::set_rom(unsigned short address, char value)
{
	rom[address] = value;
}

