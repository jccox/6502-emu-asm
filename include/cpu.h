#ifndef CPU_H
#define CPU_H

#define DEBUG

struct reg{
	reg(void) {xreg = yreg = areg = flags = 
			fc=fz=fi=fd=fb=fv=fn  =
			pc = stackp = cyclecount = 0;}
	char xreg;
	char yreg;
	char areg;
	unsigned char flags;		 //Flag register packed.
	char fc, fz, fi, fd, fb, fv, fn; //Flag register unpacked.
	unsigned short pc; 		 //init to (0xfffc)
	unsigned char stackp;
	unsigned int cyclecount;
};

typedef struct reg regs;

enum interrupt_type{
	NMI = 0x01, NMI2 = 0x02, MI = 0x04
};

enum flag_names{
	C, Z, I, D, B, W, V, N
};

enum flag_type{
	CARRY, INT, OVERFLOW, DECI
};
	
enum register_type{
	RA, RX, RY, RS, RPC, RF
};

enum shift_t{
	RLEFT, RRIGHT, ALEFT, LRIGHT
};

enum incdec_t{
	INC_T, DEC_T
};



enum program_states{
	FETCH_PROG, READ_PROG, RUN_INST
};

enum instruction_type{
	READ, WRITE, RW, 
};

enum instruction_finish{
	CONTINUE, DONE, IMPLDONE
};


enum onebyte_opcodes{
	PHA = 0x48, PHP = 0x08, PLA = 0x68, PLP = 0x28, DEX = 0xCA,
	DEY = 0x88, INY = 0xC8, INX = 0xE8, TAY = 0xA8, TYA = 0x98,
	TXS = 0x9A, TXA = 0x8A, TAX = 0xAA, TSX = 0xBA, NOP = 0xEA,
	BRK = 0x00, JSRA = 0x20, RTI = 0x40, RTS = 0x60
};


enum addr_mode{
	ACUM, ABS, ABSX, ABSY, IMED, IMPL, IND, XIND, INDY, REL, ZPG, ZPGX, ZPGY
};


class cpu6502{

	//Opcode operations
	void adcf(void);
	void andf(void);
	void bit(void);
	void branch(int type);	
	void brk(void);
	void cmpf(int reg);
	void eorf(void);
	void incdecf(int type);
	void jmp(void);
	void jsr(void);
	void oraf(void);
	void rti(void);
	void rts(void);
	void sbcf(void);
	void shiftf(int dir);
	void one_byte(void);
	void load(int reg);
	void store(int reg);

	int get_memsize(void);
	int get_type(void);
	void get_address(void);
	void fetchread(void);
	void do_work(void);
	void opdecode(void);
	void pack_flags(void);
       	void unpack_flags(void);

	int addr_mode;
	
		

protected:
	regs registers;

	int get_addr_mode(void);
	
	
	int opsize;
	int fetched_data;
	int opmode;
	unsigned char opcode;
	unsigned short address_value;
	int instruction_state;
	virtual unsigned char mem_read(unsigned short address) = 0;
	virtual void mem_write(unsigned short address, char value) = 0;

	int get_cycles(void);


	int cycles_total;

	int cycles;
	int interrupt_cycle;
	int interrupt;
	
public:
	cpu6502(void);
	void cpureset(void);

	//void reload_memory(char *ram);
	void run_instruction(void);
	void set_interrupt(void);
	void set_nmi(void);

};

#endif //CPU_H
