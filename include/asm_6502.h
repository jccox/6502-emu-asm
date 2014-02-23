#ifndef ASM_6502_H
#define ASM_6502_H

#include <search.h>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cctype>
//#include "assembler.h"
#include "list.h"



//"$XXXX\0" == 6 chars as the string size
//of a 16 bit address.
#define ADDRESS_SIZE 6


#define ON 1
#define OFF 2

#define printer(str, line) printerror(str, line);					\
			       exit(1);
#define printe(str, src, line) sprintf(errormsg, str, src); \
	                       printerror(errormsg, line);					\
			       exit(1); 

/*This is used to hold user defined constants.
  The type is of WORD_TYPE. The value is an address
  string of size ADDRESS_SIZE if a label, a number string
  if a value, etc.*/
struct subword{
	int type;
	char *value;
};


enum WORD_TYPE{
	LABEL_T, MACRO_T, VALUE_T, OBJ_T
};


class asm_6502{
	FILE *in;
	char *out;

	int i;
	int j;
	int linecount;
	static const char * const iset[];	
	static const int isetmodes[]; 
			   

	static const char * const addr_modes[];
	static const char * const addr_mode_names[];
					
	/*Assembler directives.*/
	static const char * const directives[];

	int reset_flag;

	int help;
	char *macrostring;
	char opcode_byte;
	int opcode_index;
	int directive_index;
	int macstatus;
	int addr_mode;
	int digitcount;
	char digitvalue[4];

	int bufint;
	char buf[1000];
	char errormsg[100];

	char *lineloc;
	char *linelocb;
	char *current_word;
	char *tokenword;
	void *searchloc;
	void *addressloc;
	char *cbuf;

	int total;
	int current_location;

	int program_size;

	struct subword *data_array; 
	struct subword *data_origin;
	unsigned char data_count;

	ENTRY entrybuf;
	ENTRY *enterreturn;

	int wordcount;
	struct List_T *lines;
 unsigned char output[0x8000];	

	void reset_assembler(void);
	void put_data(void);
	void put_data(FILE *in, int linenumber);
	void first_pass(void);
	void second_pass(void);
	void third_pass(void);
	unsigned char opcode_map(int opcode_index, int addr_mode);
	void end_assemble(void);
	struct subword *get_word(char *key); 
	int get_program_size(void);

public:
	asm_6502();
	~asm_6502();
	
	void assemble(FILE *input, char *output);


	/*Let a debugger get information about
	  an assembled program.*/
	friend class debug;

};


void stoupper(char *string);
int atohex(char convert);
void printerror(const char *string, int line);
void printwarning(const char *string, int line);
int strtohex(char *str);
void free_str(void *str);

#endif //ASM_6502_H
