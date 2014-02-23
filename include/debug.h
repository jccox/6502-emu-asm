#ifndef DEBUG_H
#define DEBUG_H


class debug : public jpc{
	char *mnemonic;
	int original_address;

public:
	
	void debug_instruction(void);
	void get_mnemonic(unsigned char opcode, char *mnemonic);
	void print_address(char *out);
	int get_opcode(void);
	int get_opsize(void);
	int get_total_cycles(void);
	unsigned char get_mem(unsigned short address);
	unsigned char get_registers(int register_type);
	unsigned short get_pc(void);
	void print_io_buffer(void);
	int get_prog_size(void);
	
	int get_flag(int flag_type);

	int map_label(char *label);
};

#endif //DEBUG_H
