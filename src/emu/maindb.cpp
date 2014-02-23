#include "include.h"
#include "optparse.h"

int atohex(char convert);
int strtohex(char *str);

void help(void);

int main(int argc, char *argv[])
{
  //        set_usage("Usage: emu input.  Do emu -h for help.\n");
	
	if(argc < 2){
	  printf("Usage emu input");
	  //	        call_usage();
		return 0;
	}	

	FILE *in = fopen(argv[1], "r");
	debug pc;

	char buf[0x8000];
	char address_out[0x8000];
	char *parser = NULL;
	
	char *instruct = new char[100];
	
	int address_jump = 0;

	bool jumping = false;
	
	FILE *title = fopen("title.rc", "r");
	
	/*Print the title banner.*/
	printf("\n");
	if(title != NULL){
	        while(fgets(buf, 0x8000, title))
		        printf("%s", buf);
		fclose(title);
	}
	/*Assemble the program from in into a buffer.*/
	pc.assemble(in, buf);	

	pc.load_rom(buf);

	pc.cpureset();	
	   
	printf("\nEnter help for help.  Press enter to step.\n");

	while(1){
		pc.debug_instruction();
		if(jumping == true && pc.get_pc() != address_jump)
		        continue;
		jumping = false;
		
		pc.get_mnemonic(pc.get_opcode(), instruct);
		printf("%s :", instruct);
		pc.print_address(address_out);
		/*Program status.*/
		printf("%s\n", address_out);
		printf("A = %x, X = %x, Y = %x\n", pc.get_registers(RA), pc.get_registers(RX), pc.get_registers(RY));
		printf("PC = %x, (PC) = %x, opcode = %x, opsize = %d, SP = %x\n", pc.get_pc(), pc.get_mem(pc.get_pc()), pc.get_opcode(), pc.get_opsize(), pc.get_registers(RS));
		printf("C = %d, Z = %d, I = %d D = %d, ", pc.get_flag(C), pc.get_flag(Z), pc.get_flag(I), pc.get_flag(D));
		printf("B = %d, V = %d, N = %d\n\n\n\n", pc.get_flag(B), pc.get_flag(V),pc.get_flag(N));
		
		/*Command prompt.*/
		while(1){
		        printf(">> ");
			fgets(buf, 100, stdin);
			if(buf[0] == '\n')
			        break;
			
			/*If it is not a continue,
			  strip the newline.*/
			buf[strlen(buf) - 1] = '\0';
		  
		  
			parser = strtok(buf, " ");

			
			if(strcmp(parser, "print") == 0){
			        parser = strtok(NULL, " ");
				if(strcmp(parser, "mem") == 0){
				        parser = strtok(NULL, " ");						    
					printf("%x, %x\n", strtohex(parser), pc.get_mem(strtohex(parser)));
				}
				
				else if(strcmp(parser, "cycles") == 0){
				        printf("%d\n", pc.get_total_cycles());
				}
				else if(strcmp(parser, "size") == 0){
				        printf("%d bytes\n", pc.get_prog_size());
				}
				
			}
			else if(strcmp(parser, "break") == 0){
			        goto brpt;
			}
			else if(strcmp(parser, "stop") == 0){
			brpt:;
			        parser = strtok(NULL, " ");
				if(strcmp(parser, "at") == 0){
			                parser = strtok(NULL, " ");
					stoupper(parser);
					address_jump = pc.map_label(parser);
					if(address_jump == -1){
					        printf("Error: Label %s does not Exist.\n", parser);
						continue;
					}
					/*If you gave a valid label, start skipping the debugging.*/
					jumping = true;
					break;
				}
			}
			else if(strcmp(parser, "help") == 0)
			        help();
			else if(buf[0] == 'q' || strcmp(parser, "exit") == 0 || strcmp(parser, "quit") == 0)
			        goto exit;
		}
	}

 exit:;
	delete []instruct;	

	fclose(in);
	return 0;		

}

void help(void)
{
        char buf[100];
       
	printf("Help: Enter a command for help or exit to go back\n");
	printf("Commands:\n");
	printf("print mem x\n");
	printf("print cycles\n");
	printf("print size\n");
	printf("stop at x\n\n");
	printf("break x\n")''
	while(1){
	  
	  fgets(buf, 100, stdin);
	  buf[strlen(buf) - 1] = '\0';
	  
	  if(strcmp(buf, "print mem x") == 0)
	    printf("Print the value at address x.\n");
	  else if(strcmp(buf, "print cycles") == 0)
	    printf("Print the number of cycles that have passed.\n");
	  else if(strcmp(buf, "print size") == 0)
	    printf("Print the size of the program (User defined data and program memory).\n");
	  else if(strcmp(buf, "stop at x") == 0 || strcmp(buf, "break x") == 0)
	    printf("Run the program without the debugger until you reach the label x.\n");
	  else if(strcmp(buf, "exit") == 0 || strcmp(buf, "quit") == 0)
	    break;
	}
}

