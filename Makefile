CPPC = g++

INCLUDES = -I ./include

FLAGS = $(INCLUDES) -Wall -g
C_FLAGS = $(INCLUDES) -Wall -g -c

VPATH = src/emu:src/util:src/asm:include

CCMP = 	@ $(CPPC) $(C_FLAGS) $< 
CMP = @$(CPPC) $(FLAGS) $< *.o -o emu

all: emu

emu: main.cpp asm_6502 cpu io jpc memory include.h list
	$(CMP)
	@rm *.o

asm_6502: asm_6502.cpp asm_6502.h
	$(CCMP)

cpu: cpu.cpp operations cpu.h
	$(CCMP)

operations: operations.cpp cpu.h
	$(CCMP)

io: io.cpp io.h
	$(CCMP)

jpc: jpc.cpp jpc.h
	$(CCMP)

list: list.c list.h
	$(CCMP)

memory: memory.cpp memory.h
	$(CCMP)


debug: maindb.cpp asm_6502 cpu debug_c io jpc memory include.h list
	$(CMP)
	@rm *.o

debug_c: debug.cpp debug.h
	$(CCMP)

clean:
	rm emu *.o
