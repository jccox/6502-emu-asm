#ifndef MEMORY_H
#define MEMORY_H


class memory32k{
	char *ram;
	char *rom;
	void set_rom(unsigned short address, char value);
protected:
	char get_rom(unsigned short address);
	char get_ram(unsigned short address);
	void set_ram(unsigned short address, char value);
public:
	memory32k(FILE *in);
	memory32k(void);
	~memory32k(void);
	void load_rom(char *image);
};

#endif //MEMORY_H
