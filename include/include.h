#ifndef INCLUDE_H
#define INCLUDE_H


using namespace std;

#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <curses.h>
#include "cpu.h"
#include "io.h"
#include "asm_6502.h"
#include "memory.h"
#include "jpc.h"

#include "debug.h"


#define HIGH_NIB(x) (x >> 4)
#define getbit(x, bit) ((x & (1 <<bit)) >> bit)
#define setbit(x, bit) (x |= (1 << bit))
#define clearbit(x, bit) (x &= ~(1 << bit))
#define stackpush(byte) mem_write(registers.stackp | 0x100, (byte)); --registers.stackp
#define stackpop(byte) ++registers.stackp; stackpeek(byte)
#define stackpeek(byte) byte = mem_read(registers.stackp | 0x100)


#endif //INCLUDE_H
