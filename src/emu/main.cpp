#include "include.h"
#include "optparse.h"

int main(int argc, char *argv[])
{
  //        set_usage("Usage: emu input.  Do emu -h for help.\n");


	if(argc < 2){
	  printf("Usage emu input");
	  //	        call_usage();
		return 0;
	}

	FILE *in = fopen(argv[1], "r");
	
	if(in == NULL){
		printf("Error.  File '%s' does not exist.\n", argv[1]);
		return 1;
	}


	jpc pc;

	char buf[0x8000] = {0};

	
	/*Assemble the program from in into a buffer.*/
	pc.assemble(in, buf);	

	//Load the buffer to rom.
	pc.load_rom(buf);
	
	pc.cpureset();	

	fclose(in);

	//Main loop!
	while(1){
		pc.run_instruction();

	}	

	return 0;
}
