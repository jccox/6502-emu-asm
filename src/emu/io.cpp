/**
 * @file io.cpp
 * 
 * @author Joseph Cox
 * 
 * @brief This file holds functions allowing IO with the linux file system. 
 *
 * @todo Null
 *
 * @bugs None.
 */


#include "include.h"


linuxio::linuxio(void)
{
	//Note that port 0 feeds back into the program.  Therefore, it
	//does not actually output to a file.
	port[0] = NULL;
	port_dir[0] = WRITE;

	port[1] = stdin;
	port_dir[1] = READ;
	port[2] = stdout;
	port_dir[2] = WRITE;
	for(int i = 3; i < 8; ++i){
		port[i] = NULL;
		port_dir[i] = 0;
	}

	for(int i = 0; i < 0x100; ++i)
	        buf[i] = 0;
	
	linebuf_enable = 0;
	force_default = 0;
	force_output = 0;
	newio = 0;
	newi_newo = 0;
	portno = 0;

	bufval = 0;
	last_port = 0;
}

linuxio::~linuxio(void)
{
	int i;
	
	for(i = 3; i < 0x8; ++i)
		if(port[i] != NULL)
			fclose(port[i]);
}
/**
 * @param unsigned short address The address received from the
 * integration class.
 * @param char value The byte to write to memory.
 */

void linuxio::writedecoder(unsigned short address, char value)
{
	if(address % 2 == 0)
		controlreg(value);
	else
		gpoutreg(value);
}

/** 
 * @param value The byte to set the control reg to.
 * @remarks Here is what the bits do to the control reg.
 * Bit 0 - Turn on to force buffer to output to last used port.
 * Note that this cancels all control features after this.
 * Bit 1 - Turn on default forced output (Output characters at a time)
 * Bit 2 - Line buffering enabled (Active low).
 * Bit 3 - Determine whether port is being defined
 * Bit 4 - Determine whether defined port is input(0) or output(1).  This is only useful
 * for port 0, which assigns files to ports.
 * Bits 5-7 - Select which port to use.
 *	   
**/
void linuxio::controlreg(char value)
{
	force_output = (value & 0x1);
	if(force_output){
		gpout();
		return;
	}

	force_default = (value & 0x2) >> 1;

	linebuf_enable = (value & 0x4) >> 2;

	newio = (value & 0x8) >> 3;

	newi_newo = (value & 0x10) >> 4;

	portno = (value & 0xE0) >> 5;

}
/**
 * @return char The byte read from the port.
 */


char linuxio::gpinreg(void)
{
	if(port_dir[portno] != READ){
		printf("Error: Port %d has wrong direction!\n", portno);
		exit(1);
	}
	return fgetc(port[portno]);
		
}

/**
 * @param char value The byte to write to the port.
 */


void linuxio::gpoutreg(char value)
{
	buf[bufval++] = value;
	
	if(force_default || value == '\0' || (value == '\n' && linebuf_enable))
		gpout();

}

/**
 * @description This function either opens up a new port or writes to an open port.
 */

void linuxio::gpout(void)
{
  
        bufval = 0;
        //This statement opens up a file pointer to some file if selected by the control reg.
	if(newio){
		if(portno <= 2){
			printf("Error: ports 0, 1, and 2 are reserved!");
			exit(1);
		}
		if(port[portno] != NULL)
			fclose(port[portno]);
		if(buf[strlen(buf) - 1] == '\n')
			buf[strlen(buf) - 1] = '\0';
		port_dir[portno] = newi_newo;
		
		port[portno] = ((port_dir[portno] == READ)? fopen(buf, "r") : fopen(buf, "w"));
		if(port[portno] == NULL){
			printf("Error: Port %d has bad name!\n", portno);
			exit(1);
		}
	}
	else{
		if(port_dir[portno] != WRITE){
			printf("Error: Port %d has wrong direction!\n", portno);
			exit(1);
		}
		fprintf(port[portno], "%s", buf);
		
		/*Flush the buffer after it is finished
		  printing.*/
		for(int i = 0; i < 0x100; ++i)
		        buf[i] = 0;
	}	
}

void linuxio::get_buffer(char *new_buffer)
{
        strcpy(new_buffer, buf);
}
