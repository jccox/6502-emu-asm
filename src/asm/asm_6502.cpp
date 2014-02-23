#include "asm_6502.h"
//#include "assembler.h"

namespace assembler_constants
{
	/* 
	   An enumeration of the instruction set's index w.r.t.
	   asm_6502::iset.
	   Note that setcard is the cardinality of the instruction set.
	*/

	enum i_set_element{
		ADC, AND, ASL, BCC, BCS, BEQ,
		BIT, BMI, BNE, BPL, BRK, BVC,
		BVS, CLC, CLD, CLI, CLV, CMP,
		CPX, CPY, DEC, DEX, DEY, EOR,
		INC, INX, INY, JMP, JSR, LDA,
		LDX, LDY, LSR, NOP, ORA, PHA,
		PHP, PLA, PLP, ROL, ROR, RTI,
		RTS, SBC, SEC, SED, SEI, STA,
		STX, STY, TAX, TAY, TSX, TXA,
		TXS, TYA, 
		
		setcard
	};

	//IMPL is not used in calculations, so addrcard skips it.
	enum addr_mode{
		IMED, ZPG,  ZPGX, ZPGY, ABS,  ABSX,
		ABSY, INDY, XIND, IND,  ACUM, addrcard,
		IMPL
	};
	
	/*This indexes to the assembler directive strings.*/
	enum dir_index{
		ALIGN, BEGIN,   BYTE,  BYTE1, END,  ENDMAC,
		EQU,   INCLUDE, MACRO, ORG,   TEXT, WORD, 
		dircard
	};


}


using namespace assembler_constants;

static int scmp(const void *a, const void *b);

const char * const asm_6502::iset[] = { "ADC" ,"AND", "ASL", "BCC", "BCS", "BEQ",
					"BIT", "BMI", "BNE", "BPL", "BRK", "BVC",
					"BVS", "CLC", "CLD", "CLI", "CLV", "CMP",
					"CPX", "CPY", "DEC", "DEX", "DEY", "EOR", 
					"INC", "INX", "INY", "JMP", "JSR", "LDA", 
					"LDX", "LDY", "LSR", "NOP", "ORA", "PHA",
					"PHP", "PLA", "PLP", "ROL", "ROR", "RTI", 
					"RTS", "SBC", "SEC", "SED", "SEI", "STA",
					"STX", "STY", "TAX", "TAY", "TSX", "TXA",
					"TXS", "TYA"};
					
const char * const asm_6502::directives[] = { ".ALIGN", ".BEGIN",  ".BY",   ".BYTE", 
					      ".END",   ".ENDMAC", ".EQU",  ".INCLUDE", 
					      ".MACRO", ".ORG",    ".TEXT", ".WORD"};

/*This gives the allowed address modes for the corresponding instruction (By alphabet).
  It is in one-hot form, with the LSB corresponding to the first entry
  in the addr_mode enum and going up from there. Note that relative addresses
  are treated as zeropages.
  For example, with ADC, we have the binary code 1 1111 0111.  This corresponds to
  the following addressing modes: IMED, ZPG, ZPGX, ABS, ABSX, ABSY, INDY, and
  XIND.*/  	

const int asm_6502::isetmodes[] = {0x01F7, 0x01F7, 0x0436, 0x0002, 0x0002, 0x0002,
				   0x0012, 0x0002, 0x0002, 0x0002, 0x1000, 0x0002,
				   0x0002, 0x1000, 0x1000, 0x1000, 0x1000, 0x01F7,
				   0x0013, 0x0013, 0x0036, 0x1000, 0x1000, 0x01F7,
				   0x0036, 0x1000, 0x1000, 0x0210, 0x0010, 0x01F7,
				   0x005B, 0x0037, 0x0436, 0x1000, 0x01F7, 0x1000,
				   0x1000, 0x1000, 0x1000, 0x0436, 0x0436, 0x1000,
				   0x1000, 0x01F7, 0x1000, 0x1000, 0x1000, 0x01F6,
				   0x001A, 0x0016, 0x1000, 0x1000, 0x1000, 0x1000,
				   0x1000, 0x1000}; 


//These are what the allowed addressing modes look like.  When checking for valid modes,
//all numbers are converted to generic X's and compared with these strings.  For example,
//if we have the address $3234, when it is tested it will have become $XXXX.  However,
//if we have $323, it will have another X appended, becoming $XXXX instead of $XXX.

const char * const asm_6502::addr_modes[] = { "#$XX",     "$XX",     "$XX,X",  "$XX,Y", 
					      "$XXXX",    "$XXXX,X", "$XXXX,Y","($XX),Y",
					      "($XX,X)",  "($XXXX)", "A"};


/*This maps asm_6502::addr_modes to each mode's actual name.*/
const char * const asm_6502::addr_mode_names[] = { "IMED", "ZPG", "ZPGX", "ZPGY", 
						   "ABS", "ABSX", "ABSY", "INDY", 
						   "XIND", "IND", "ACUM", "", 
						   "IMPL"};


char get_esc(char value);

asm_6502::asm_6502()
{
        in = NULL;
	out = NULL;
	i = 0;
	j = 0;
	linecount = 0;
	
	reset_flag = 0;
	help = 0;
	macrostring = NULL;
	opcode_byte = 0;
	directive_index = 0;
	macstatus = 0;
	addr_mode = 0;
	digitcount = 0;

	for(i = 0; i < 0x10; ++i)
	       digitvalue[i] = 0;

	bufint = 0;
	for(i = 0; i < 1000; ++i)
	        buf[i] = 0;
	
	for(i = 0; i < 100; ++i)
	        errormsg[i] = 0;

	linelocb = new char[0x100];
	
	current_word = NULL;
	tokenword = NULL;
	searchloc = NULL;
	addressloc = NULL;
	total = 0;
	current_location = 0;
	program_size = 0;
	data_count = 0;

	data_array = new struct subword[0x100];
	for(i = 0; i < 0x100; ++i){
	        data_array[i].value = new char[0x100];
		for(j = 0; j < 0x100; ++j)
		        data_array[i].value[j] = 0;
	}
	data_origin = NULL;
	
	entrybuf.key = new char[0x40];

	lines = list_new();
	list_reset(lines);	

}

asm_6502::~asm_6502()
{
        delete []linelocb;

	for(i = 0; i < 0x100; ++i)
		delete [](data_array[i].value);

	
	delete []data_array;
	delete []entrybuf.key;

	list_free(&lines, free_str);

	if(reset_flag == 1)
	        hdestroy();
	

}

void asm_6502::reset_assembler(void)
{
        //Build a new hash if you've already assembled.
        if(reset_flag == 1)
	        hdestroy();
  
        
	reset_flag = 1;
	help = 0;
	macrostring = NULL;
	macstatus = 0;
		
	current_word = NULL;
	tokenword = NULL;
	searchloc = NULL;
	addressloc = NULL;
	
	current_location = 0;
	data_origin = data_array;

	total = 0;

	enterreturn = NULL;

	entrybuf.data = data_array;

	wordcount = 0;

	linecount = 0;

	for(i = 0; i < 0x8000; ++i)
	  output[i] = 0;

	for(i = 0; i < 4; ++i)
	  digitvalue[i] = -1;

	/*Reset all of the assembler's data strings.*/
	for(i = 0; i < data_count; ++i)
	        data_array[i].value[0] = '\0';
	data_count = 0;

	
	program_size = 0;

	list_free(&lines, free_str);
	lines = list_new();
	list_reset(lines);
	hcreate(0x100);


}

void asm_6502::assemble(FILE *input, char *output)
{
        //Get the assembler to a reusable condition.
        reset_assembler();
  	
	in = input;
	out = output;
	if(in == NULL || out == NULL){
		printf("Asm_6502: Error: File was NULL");
		exit(1);
	}

	put_data();
	first_pass();
	second_pass();
	third_pass();
	end_assemble();
}

void asm_6502::put_data(void)
{
  put_data(in, 0);
}

void asm_6502::put_data(FILE *in, int line_num)
{
        char *line = NULL;
	unsigned char tmp;
	char num_buf[10];	
	for(i = line_num; fgets(buf, 1000, in); ++i){

	  
	  if(buf[strlen(buf) - 1] == '\n')buf[strlen(buf) - 1] = '\0';	

		//Get to a comment if there is one
		for(j = 0; buf[j] != ';' && buf[j] != '\0'; ++j);

		//Turn it to a null terminator.
		buf[j] = '\0';

		//Get rid of whitespace at the end.
		for(j = (int) strlen(buf) - 1; buf[j] == ' ' || buf[j] == '\t'; --j)
			buf[j] = '\0';
		//Replace ascii with hex values
		for(j = 0; j < (int) strlen(buf); ++j)
		  if(buf[j] == '\''){
		          if(buf[j+2] == '\'' || (buf[j+1] == '\\' && buf[j+3] == '\'')){
			          if(buf[j+1]  == '\\')
				          tmp = get_esc(buf[j+2]);
				  else
				          tmp = get_esc(buf[j+1]);
				  sprintf(num_buf, "$%02x", tmp);
				  strcpy(buf + j, num_buf);
			  }
			  else{			    
			          printe("%s has a bad ASCII character.", buf, i);
			  }
		  }
		//Upper case all non text sequences
		for(j = 0; j < (int) strlen(buf); ++j){
			if(isalpha(buf[j]))
				buf[j] = toupper(buf[j]);
			else if(buf[j] == '\"')
				break;
		}
		line = new char[strlen(buf) + 1];
		strcpy(line, buf);
		list_put(line);
		list_next();
	}

	total += (i + 1) - line_num;
}

/**
 * @description On the first pass, we do macros and other preprocessor
 * directives.
 *
 */
void asm_6502::first_pass(void)
{
 begin:;
       list_reset(lines);
       /*Put back onto the listing and increment every cycle.*/
       for(linecount = 0 ;(cbuf = (char *)list_get());list_next(), ++linecount){
	
	  strncpy(linelocb, cbuf, 0x100);
	  lineloc = linelocb;

	       //Increment past whitespace at the beginning.
	       while(lineloc[0] == '\t' || lineloc[0] == ' ')
		       ++lineloc;
	       
	       if(lineloc[0] == '\0')
			continue;
		

		current_word = strtok(lineloc, " ");


		if(strcmp(current_word, ".ENDMACRO") == 0){
			macstatus = OFF;
			entrybuf.data = &(data_array[data_count++]);
			hsearch(entrybuf, ENTER);
			cbuf[0] = '\0';
		}
		else if(strcmp(current_word, ".INCLUDE") == 0){
		        current_word = strtok(NULL, " ");
			//Check for argument.
			if(current_word == NULL){
			  	printe("%s is not a file name.", current_word, linecount);
			}
			//Check for proper format of the argument.
			if(current_word[0] != '\"' || current_word[strlen(current_word) - 1] != '\"'){
			        printe("%s must be enclosed in quotes; single for internal, double for external.", current_word, linecount);
			}
			//Get rid of the quotes.
			++current_word;
			current_word[strlen(current_word) - 1] = '\0';
			//Open the file and check for being an object file.
			FILE *tmp_in = fopen(current_word, "r");
			if(tmp_in == NULL){
				printe("%s does not open.", current_word, linecount);
			}
			fgets(buf, 100, tmp_in);
			//Restart the file for put_data.
			rewind(tmp_in);			
			//If we aren't a static/dynamic object file, replace the line with the file.
			if(strncmp(buf, "6502asmSL", 9) != 0 && strncmp(buf, "6502asmSL", 9) != 0){			  
			  cbuf[0] = '\0';
			  
			  put_data(tmp_in, i + 1);
			  goto begin;
			}
			fclose(tmp_in);
		}
		//Concatenate the current line if it is inside of a macro statement.  
		//Then, turn the line to nothing.
		else if(macstatus == ON){
			lineloc[strlen(lineloc)] = '\n';
			strcat(macrostring, lineloc);
			//Disappear the current line.	
			cbuf[0] = '\0';
		}
		//Keep track of labels.
		else if(current_word[strlen(current_word) - 1] == ':'){

			current_word[strlen(current_word) - 1] = '\0';
			strcpy(entrybuf.key, current_word);

			if(hsearch(entrybuf, FIND) != NULL){
				sprintf(errormsg, "%s already defined.", current_word);
				printwarning(errormsg, linecount);
			}

			else if(bsearch(current_word, iset, setcard, sizeof(char*), scmp) != NULL ||
				bsearch(current_word, directives, dircard, sizeof(char*), scmp) != NULL){
			        printe("%s is an illegal label name.", current_word, linecount);
			}
			//Make a hash for the label, if the label exists.
			data_array[data_count].type = LABEL_T;
			
			entrybuf.data = &(data_array[data_count++]);
			hsearch(entrybuf, ENTER);
		}
		//Skip over the instruction set.
		else if(bsearch(current_word, iset, setcard, sizeof(char*), scmp) != NULL){
			continue;
		}
		//Check for infix directives acting as prefix directives.  This is required because
		//they are the only preprocessor directives.
		else if(strcmp(current_word, ".EQU") == 0 || strcmp(current_word, ".MACRO") == 0){
		        printe("%s is not a prefix directive.  Please give a prefix argument.\n", current_word, linecount);
		}
		else if(bsearch(current_word, directives, dircard, sizeof(char*), scmp) != NULL){		
			continue;
		}
		//If it has not been found yet, then it must be a prefix to the preprocessor
		//directives.
		else{
			strcpy(entrybuf.key, current_word);
			current_word = strtok(NULL, " ");
			
			if(current_word == NULL){
			        printe("%s is an unknown directive.\n", current_word, linecount);
			}
			if(strcmp(current_word, ".EQU") == 0){
				current_word = strtok(NULL, " ");
				if(current_word == NULL){
					printer("No postfix operand was given for the infix operator .EQU.  Please give one", linecount);
				}
				if(current_word[0] != '$'){
						printer(".EQU only accepts a hex value ($XXXX or $XX) as an argument.", linecount);
				}
				for(i = 1; i < (int) strlen(current_word); ++i)
				        if(!(isdigit(current_word[i]))){
						printer(".EQU only accepts a hex value ($XXXX or $XX) as an argument.", linecount);
					}
				data_array[data_count].type = VALUE_T;	
				strcpy(data_array[data_count].value, current_word);

				entrybuf.data = &(data_array[data_count++]);
				hsearch(entrybuf, ENTER);
				

				cbuf[0] = '\0';
			}
			else if(strcmp(current_word, ".MACRO") == 0){
				data_array[data_count].type = MACRO_T;	

				macrostring = data_array[data_count].value;
				macstatus = ON;

				cbuf[0] = '\0';
			}
			else{
				printe("%s is not a preprocessor infix directive.  Please give a proper directive.\n", current_word, linecount);
			}
		}
	}


	if(macstatus == ON)
		printf("Warning: Macstatus was left on from first pass\n");

}

/**
 * @description On the second pass, we count bytes and figure
 * out where labels and stuff point to.
 **/

void asm_6502::second_pass(void)
{
	current_location = 0;

	list_reset(lines);
       /*Put back onto the listing and increment every cycle.*/
	for( linecount = 0;(cbuf = (char *)list_get()); list_next(), linecount++){

	    strncpy(linelocb, cbuf, 0x100);
	    lineloc = linelocb;      
	       //Increment past whitespace

	       while(lineloc[0] == '\t' || lineloc[0] == ' ')
		       ++lineloc;

	       if(lineloc[0] == '\0')
			continue;

	       current_word = strtok(lineloc, " ");

		/*Check for labels, denoted by a colon.  */
		if(current_word[strlen(current_word) - 1] == ':'){
		  
		        /*Make the word into a key for the hash.*/
			strcpy(entrybuf.key, current_word);
			entrybuf.key[strlen(entrybuf.key)-1] = '\0';

			/*Store the location as ascii in buf and stick
			  it in a hash.*/

			sprintf(buf, "$%04x", current_location + 0x8000);
			for(i = 0; buf[i] != '\0'; ++i)
				if(isalpha(buf[i]))
					buf[i] = toupper(buf[i]);
			
			
			enterreturn = hsearch(entrybuf, FIND);
			strcpy((((struct subword *) (enterreturn->data))->value), buf);
			enterreturn = hsearch(entrybuf, FIND);
			
			/*Get rid of the label's line.*/
			cbuf[0] = '\0';
		}	

		/*If it is an instruction, increment the location counter.*/
		else if(bsearch(current_word, iset, setcard, sizeof(char*), scmp) != NULL){
		  
		        /*All instructions have at least one byte.*/
			++current_location;
			++program_size;

			current_word = strtok(NULL, " ");
	
			/*If it is an implied address mode,
			  then it is only one byte.*/
			if(current_word == NULL)
				continue;


			//Get the string which could be a label/value.  Copy the label/value.
			
			/*This if/else block will "pull" the label/values
			  out of instructions, even if they are embedded
			  inside of them.*/
			if((tokenword = strchr(current_word, ')'))){
				tokenword[0] = '\0';
				strcpy(entrybuf.key, current_word + 1);
				buf[0] = '(';
				tokenword[0] = ')';
			}

			else if((tokenword = strchr(current_word, ','))){							
				tokenword[0] = '\0';
				strcpy(entrybuf.key, current_word);
				tokenword[0] = ',';
			}
			
			else{
				tokenword = NULL;
				strcpy(entrybuf.key, current_word);
			}

			//If the value is really a label, check for the size of the
			//address (Zeropage or 2 bytes) and increment the location by that.
			if((enterreturn = hsearch(entrybuf, FIND)) != NULL){
			  
			        /*Note that you can't stick program into the zeropage!
				  That implies that you can't stick a label there!*/
				if(((struct subword *) (enterreturn->data))->type == LABEL_T){
					current_location += 2;
					program_size += 2;
				}
				else if(((struct subword *) (enterreturn->data))->type == VALUE_T){
					tokenword = ((struct subword *) (enterreturn->data))->value;

					bufint = strtohex(tokenword + 1);
					/*If zeropage*/
					if(bufint < 0x100){  
					        current_location += 1;
						program_size += 1;
					}
					else{		       
					        current_location += 2;
						program_size += 2;
					}
				}
				else{
					printer("Macros are not allowed as values!\n", linecount);
				}
			}
			else{
			        //If the argument is $XXXX
			        if(strlen(current_word) == 5){
				        current_location += 2;
					program_size += 2;
				}
				//If it is accumulator addressing:
				//We don't want to increment if it
				//is accumulator addressing.
				else if(strcmp(current_word, "A") == 0){
				        continue;
				}
				//If the argument is $XX
				else{
				        current_location += 1;
					program_size += 2;
				}
			}
			
		}

		else if((searchloc = bsearch(current_word, directives, dircard, sizeof(char*), scmp)) != NULL){

			//Get the index of the returned search.
			directive_index = ((unsigned long long) searchloc - (unsigned long long) directives) / sizeof(char *);
			
			current_word = strtok(NULL, "\0");
			if(current_word == NULL){
				sprintf(errormsg, "%s is missing an argument.\n", directives[directive_index]);
				printerror(errormsg, linecount);
				exit(1);
			}

			switch(directive_index){
			case BYTE:
			case BYTE1:
				current_word = strtok(current_word, ", ");
				do{
					++current_location;
				} while((current_word = strtok(NULL, ", ")));
				break;
			case ORG:
				if(current_word[0] != '$'){
					printer(".ORG expects a hexadecimal address (e.g. $XXXX).\n", linecount);
				}
				sscanf(current_word+1, "%x", &current_location);
				if(current_location >= 0x10000)
					printwarning("Max address size exceeded.  Modulating to between $8000 and $10000.", linecount);
				else if(current_location <  0x8000){
					if(current_location < 0){
						printer("Negative addresses do not exist.\n", linecount);
					}
					else{
						printwarning("Address less than minimum address value.  Modulating to between $8000 and $10000.\n", linecount);
					}
				}
				//Note that the maximum location is $10000, but current location is offset to go to $8000.
				current_location &= 0x7FFF;
				break;
				
			case TEXT:
				if(!(current_word[0] == '"' && current_word[strlen(current_word) - 1] == '"')){
					printer(".TEXT strings expect double quotes on both ends.\n", linecount);
				}
				++current_word;
				/*Get rid of the quotes.*/
				current_word[strlen(current_word) - 1] = '\0';

				for(i = 0; current_word[i] != '\0'; ++i)
					if(current_word[i] != '\\')
						++current_location;
				++current_location;
				break;
			case WORD:
							    
			        current_word = strtok(current_word, ", ");

				do{
					current_location += 2;
				} while((current_word = strtok(NULL, ", ")));
				break;
			default:
				break;
			}	
		}
	}
}

/**
 * @description On the third pass, we fill in a memory buffer with the program.
 *
 **/

void asm_6502::third_pass(void)
{	
        list_reset(lines);
	/*Put back onto the listing and increment every cycle.*/
	for(linecount = 0;(cbuf = (char *)list_get()); list_next(), linecount++){
	  
     
	  strncpy(linelocb, cbuf, 0x100);
	  lineloc = linelocb;
     
		//Increment past whitespace

		while(lineloc[0] == '\t' || lineloc[0] == ' ')
			++lineloc;

		if(lineloc[0] == '\0')
			continue;

		current_word = strtok(lineloc, " ");
	
		//Assembler directives
		if((searchloc = bsearch(current_word, directives, dircard, sizeof(char*), scmp)) != NULL){

			//Get the index of the returned search.
			directive_index = ((unsigned long long) searchloc - (unsigned long long) directives) / sizeof(char *);
			
			
			current_word = strtok(NULL, "\0");
			if(current_word == NULL){
				sprintf(errormsg, "%s is missing an argument.\n", current_word);
				printerror(errormsg, linecount);
				exit(1);
			}

			switch(directive_index){
			case BYTE:
			case BYTE1:
				current_word = strtok(current_word, ", ");
				do{
				  
				        if(strlen(current_word) > 3){
				                sprintf(errormsg, "%s must be a one-byte number.\n", current_word);
				                printerror(errormsg, linecount);
				                exit(1);
				        }
				        /*Skip the dollar sign in the byte and turn it into hex.*/
					if(current_word[0] == '$')
					        ++current_word;
				        output[current_location++] = strtohex(current_word);
				}while((current_word = strtok(NULL, ", ")));
				break;
			case ORG:
				sscanf(current_word+1, "%x", &current_location);
				//Note that the maximum location is $10000, but current location is offset to go to $8000.
				current_location &= 0x7FFF;
				break;
			case TEXT:
				if(!(current_word[0] == '"' && current_word[strlen(current_word) - 1] == '"')){
					printerror(".TEXT strings expect double quotes on both ends.\n", linecount);
					exit(1);
					printe("%s is not a preprocessor infix directive.  Please give a proper directive.\n", current_word, linecount);
				}
				/*Get rid of the quotes.*/
				++current_word;
				current_word[strlen(current_word) - 1] = '\0';
								
				for(i = 0; current_word[i - 1] != '\0'; ++i, ++current_location){
					//Replace Escape character sequences
					if(current_word[i] == '\\'){
						++i;
						output[current_location] = get_esc(current_word[i]);
					}
					else
						output[current_location] = current_word[i];
				}
				break;
			case WORD:
			        current_word = strtok(current_word, ", ");			
				do{
				       /*Check if the word is a label and replace it with its value if so.*/
				        strcpy(entrybuf.key, current_word);
				  
					if((enterreturn = hsearch(entrybuf, FIND)) != NULL){
					        if(((struct subword *) (enterreturn->data))->type == LABEL_T){
						        strcpy(buf, ((struct subword *) (enterreturn->data))->value);							
							current_word = buf;							
						}
					}
					if(strlen(current_word) > 5){	 				  
					        sprintf(errormsg, "%s must be a two-byte number or a label.\n", current_word);
						printerror(errormsg, linecount);
						exit(1);
					}
					/*Skip the dollar sign in the byte and turn it into hex.*/
					if(current_word[0] == '$')
					        ++current_word;
					bufint = strtohex(current_word );
					/*Output to little endian.*/
					output[current_location++] = (bufint & 0x00FF);
					output[current_location++] = (bufint & 0xFF00) >> 8;
					
				}while((current_word = strtok(NULL, ", ")));
				break;
			default:
				break;
			}
		}

		//Search and give the index of the mnemonic to searchloc.
		else if((searchloc = bsearch(current_word, iset, setcard, sizeof(char*), scmp)) != NULL){

			current_word = strtok(NULL, " ");
			
			if(current_word == NULL)
				addr_mode = IMPL;
			else{
				if((tokenword = strchr(current_word, ')'))){
				        /*Stick a value between the parenthesis.*/
				        //Null in order to grab right before the ).
					tokenword[0] = '\0';
					strcpy(entrybuf.key, current_word + 1);
					buf[0] = '(';
					tokenword[0] = ')';
				}
				else if((tokenword = strchr(current_word, ','))){							
					tokenword[0] = '\0';
					strcpy(entrybuf.key, current_word);
					tokenword[0] = ',';
				}
				
				else{
					tokenword = NULL;
					strcpy(entrybuf.key, current_word);
				}		
				if((enterreturn = hsearch(entrybuf, FIND)) != NULL){
					if(tokenword == NULL)
						strcpy(buf, ((struct subword *) (enterreturn->data))->value);
					else if(tokenword[0] == ','){
						strcpy(buf, ((struct subword *) (enterreturn->data))->value);
						strcat(buf, tokenword);
					}
					else if(tokenword[0] == ')'){
						strcpy(buf + 1, ((struct subword *) (enterreturn->data))->value);
						strcat(buf, tokenword);
					}
				}
				else{
					strcpy(buf, current_word);
				}
				current_word = buf;
				//This saves the line's address value and turns the number
				//into generic X's.

				if(current_word[0] != 'A'){ // Special case with accumulator addressing.
				        for(i = 0, digitcount = 0; current_word[i] != '\0'; ++i)
					        if(isxdigit(current_word[i])){
						        digitvalue[digitcount & 0x3] = atohex(current_word[i]);
							current_word[i] = 'X';
							++digitcount;
						}
				}
				
				else
				        digitcount = 0;
				
				if(digitcount > 4){
					printerror("Max of 4 digits exceeded.", linecount);
					exit(1);
					printe("%s is not a preprocessor infix directive.  Please give a proper directive.\n", current_word, linecount);
				}
				if(digitcount == 3 || digitcount == 1){
					i = strlen(current_word);
					current_word[i+1] = '\0';
					for(; current_word[i-1] != '$' && i != -1; --i)
						current_word[i] = current_word[i-1];
			
				}
				if((addressloc = bsearch(current_word, addr_modes, addrcard, sizeof(char*), scmp)) == NULL){
 				        sprintf(errormsg, "%s is an Invalid address type.", current_word);
					printerror(errormsg, linecount);
					exit(1);
				}
		
				addr_mode = ((unsigned long long) addressloc - (unsigned long long) addr_modes) / sizeof(char*);
				
			}
			opcode_index = ((unsigned long long) searchloc - (unsigned long long) iset) / sizeof(char*);
	
			//Error checking whether the address mode is valid.
			if((isetmodes[opcode_index] & (1 << addr_mode)) == 0){
			        sprintf(errormsg, "%s is an invalid addressing mode for given instruction.\n", addr_mode_names[addr_mode]);
				printerror(errormsg, linecount);
				exit(1);
				
			}
			opcode_byte = 0;
			
			//Figure out what the opcode is
			opcode_byte = opcode_map(opcode_index,addr_mode);
			
			output[current_location] = opcode_byte;
			++current_location;
			//If the opcode has a 2 byte argument, output the low byte(lit.endian)
			if(digitvalue[2] != -1){
				if(digitvalue[3] == -1)
					output[current_location] = digitvalue[2];
				else
					output[current_location] = (digitvalue[2] << 4) + digitvalue[3];
				++current_location;
			}
			//If the argument is 1 or 2 bytes, output the remaining byte.
			if(digitvalue[0] != -1){
				if(digitvalue[1] == -1)
					output[current_location] = digitvalue[0];
				else
					output[current_location] = (digitvalue[0] << 4) + digitvalue[1];
				++current_location;
			}
			//reset the digits for the next opcode.
			digitvalue[0] = digitvalue[1] = digitvalue[2] = digitvalue[3]  = -1;
		}

		else{
			printer("Unspecified Error\n", linecount);
		}

	}
}

unsigned char asm_6502::opcode_map(int opcode_index, int addr_mode)
{
        unsigned char opcode = 0;
	/*This switch statement takes care of the instructions which are one
	  to one or one to two with the opcodes.*/

        switch(opcode_index){
	case BCC: return 0x90;
	case BCS: return 0xB0;
	case BEQ: return 0xF0;
	case BIT:
	        if(addr_mode == ZPG)
		        return 0x24;
		else
		        return 0x2C;
	case BMI: return 0x30;
	case BNE: return 0xD0;
	case BPL: return 0x10;
	case BRK: return 0x00;
	case BVC: return 0x50;
	case BVS: return 0x70;
	case CLC: return 0x18;
	case CLD: return 0xDA;		
	case CLI: return 0x58;
	case CLV: return 0xB8;
	case DEX: return 0xCA;
	case DEY: return 0x88;
	case INX: return 0xE8;
	case INY: return 0xC8;
	case JMP:
	        if(addr_mode == IND)
		        return 0x6C;
		else
		        return 0x4C;
	case JSR: return 0x20;
	case NOP: return 0xEA;
	case PHA: return 0x48;
	case PHP: return 0x08;
	case PLA: return 0x68;
	case PLP: return 0x28;
	case RTI: return 0x40;
	case RTS: return 0x60;
	case SEC: return 0x38;
	case SED: return 0xF8;
	case SEI: return 0x78;
	case TAX: return 0xAA;
	case TAY: return 0xA8;
	case TXA: return 0x8A;
	case TXS: return 0x9A;
	case TYA: return 0x98;
	case TSX: return 0xBA;
	default:
	        break;
	}
	
	if(opcode == 0){
	        //This sets the first three bits of an opcode.
	        switch(opcode_index){						
		case ORA:    case ASL:              opcode = 0x00; break;		  
		case AND:    case ROL:              opcode = 0x20; break;		  
		case EOR:    case LSR:              opcode = 0x40; break;
		case ADC:    case ROR:              opcode = 0x60; break;
		case STA:    case STX:    case STY: opcode = 0x80; break;
		case LDA:    case LDX:    case LDY: opcode = 0xA0; break;
		case CMP:    case DEC:    case CPY: opcode = 0xC0; break;
		case SBC:    case INC:    case CPX: opcode = 0xE0; break;
		default: break;
		}
		//Next three bits!
		switch(addr_mode){
		case XIND:               opcode |= 0x00; break;
		case ZPG:                opcode |= 0x04; break;
		case IMED:    case ACUM: opcode |= 0x08; break;
		case ABS:                opcode |= 0x0C; break;
		case INDY:               opcode |= 0x10; break;
		case ZPGX:    case ZPGY: opcode |= 0x14; break;
		case ABSY:               opcode |= 0x18; break;
		case ABSX:               opcode |= 0x1C; break;
		default:
		        break;
		}
		//Get final two bits and check for exceptions
		//to addressing rules.
		switch(opcode_index){
		case ORA:    case AND:    case EOR:    case ADC:
		case STA:    case LDA:	  case CMP:    case SBC:
		  opcode |= 0x01; break;
		case ASL:    case ROL:    case LSR:    case ROR:
		case STX:    case LDX:    case DEC:    case INC:
		        if(addr_mode == IMED){
			        opcode &= ~(0x1C);
				opcode |= 0x00;
			}
			opcode |= 0x02;
			break;
		case STY:    case LDY:    case CPY:    case CPX:
		        if(addr_mode == IMED){
			        opcode &= ~(0x1C);
				opcode |= 0x00;
			}
			opcode |= 0x00;
			break;
		default:
		        break;
		  
		}
	}
	return opcode;
}

struct subword *asm_6502::get_word(char *key)
{
        strcpy(entrybuf.key, key);
	ENTRY *tmp = hsearch(entrybuf, FIND);

	if(tmp == NULL)
	        return NULL;
	
	return (struct subword *) (tmp->data);
}

int asm_6502::get_program_size(void)
{
  return program_size;
}

void asm_6502::end_assemble(void)
{
        /*Set the reset vector if it isn't already set.*/
	if(output[0x7FFC] == 0x0)
		output[0x7FFC] = 0x0;
	if(output[0x7FFD] == 0x0)	
		output[0x7FFD] = 0x80;

	/*Write the program to an output string.*/
	for(i = 0; i < 0x8000; ++i)
	        out[i] = output[i];
}

void stoupper(char *str)
{
	int i;
	
	for(i = 0; str[i] != '\0'; ++i)
		if(isalpha(str[i]))
			str[i] = toupper(str[i]);
}


int atohex(char convert)
{
	if(isdigit(convert))
		return convert - '0';
	else if(convert >= 'A' && convert <= 'F')
		return convert - 'A' + 10;
	else if(convert >= 'a' && convert <= 'f')
  	        return convert - 'a' + 10;
	else
		return 0;
}

int strtohex(char *str)
{
        unsigned int i;
	
	int total = 0;
	
	for(i = 0; i < strlen(str); ++i)
                /*Because we are doing base 16, we want to bit shift
	          4 times for every digit.*/
	        total += (atohex(str[i]) << (4 * (strlen(str) - i - 1))); 

	return total;
}

char get_esc(char value)
{
  switch(value){
  case 'n': return '\n';
  case 't': return '\t';
  case '0': return '\0';
  default:  return value;
  }

}

static int scmp(const void *a, const void *b)
{
	char *a1 = (char *)a;
	char *b1 = (char *) *((char **) b);
	return strcmp(a1, b1);
}

void printerror(const char *str, int line)
{
	printf("Error on line %d : %s\n", line, str);
}

void printwarning(const char *str, int line)
{
	printf("Warning on line %d : %s\n", line, str);
}					

void free_str(void *str)
{
  delete [](char *)str;
}
