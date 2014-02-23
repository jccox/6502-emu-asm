/*Memory mapping for the emulated device:
  0 - 0x1FFF: Ram
  0x2000 - 0x3FFF: IO mirrored every two bytes
  0x4000 - 0x7FFF: Exit the emulator
  0x8000 - 0xFFFF: Program Memory
*/



#ifndef JPC_H
#define JPC_H

class jpc: public cpu6502, public memory32k, public linuxio, public asm_6502{
	void mem_write(unsigned short address, char value);
protected:
	unsigned char mem_read(unsigned short address);
};

#endif //JPC_H
