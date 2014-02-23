/** 
 * Here is what to set the control reg to:
 * Bit 0 - Turn on to force buffer to output to last used port.
 * Note that this cancels all control features after this.
 * Bit 1 - Turn on default forced output (Output characters at a time)
 * Bit 2 - Line buffering enabled.
 * Bit 3 - Determine whether port is being defined
 * Bit 4 - Determine whether defined port is input(0) or output(1).  This is only useful
 * for port 0, which assigns files to ports.
 * Bits 5-7 - Select which port to use.
 *	   
**/



#ifndef IO_H
#define IO_H

#define READ 0
#define WRITE 1

class linuxio{
	FILE *port[0x8];
	char port_dir[0x8];
	int last_port;

	char linebuf_enable;
	char force_output;
	char force_default;
	char newio;
	char newi_newo;
	int portno;

	char buf[0x100];
	unsigned char bufval;

protected:
	void writedecoder(unsigned short address, char value);
	void controlreg(char value);
	char gpinreg(void);
	void gpoutreg(char value);
	
	void gpout(void);
	void get_buffer(char *new_buffer);
public:
	linuxio(void);
	~linuxio(void);


};

#endif //IO_H
