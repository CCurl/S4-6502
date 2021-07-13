// 6502Emu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "6502.h"

typedef unsigned char byte;
typedef unsigned short ushort;

byte mem[65536];
long cycles, lastPage;
typedef struct {
    byte a, x, y, p;
    byte sp;
    ushort pc;
} SYS_T;

#define REG_A sys.a
#define REG_X sys.x
#define REG_Y sys.y
#define FLAGS sys.p
#define SP    sys.sp
#define PC    sys.pc
#define STK   0x0100

SYS_T sys;

extern byte readByte(ushort addr);
extern void writeByte(ushort addr, byte val);

int pageChanged(ushort addr) {
    byte pg = addr >> 8;
    byte ch = (pg == lastPage);
    lastPage = pg;
    return ch ? 1 : 0;
}

 byte readByte(ushort addr) {
     ++cycles;
     if (pageChanged(addr)) { ++cycles; }
    return mem[addr];
}

void writeByte(ushort addr, byte val) {
    mem[addr] = val;
    ++cycles;
    if (pageChanged(addr)) { ++cycles; }
}

void init() {
    for (int i = 0; i <= 0xFFFF; i++) {
        writeByte(i, 0);
    }
    REG_A = REG_X = REG_Y = FLAGS = 0;
    SP = PC = lastPage = cycles = 0;
}

void push(byte val) { mem[STK+(--SP)] = val; }
byte pop() { return mem[STK+(SP++)]; }

ushort wordAt(ushort addr) {
    return readByte(addr) | (readByte(addr+1) << 8);
}

int run(ushort pc, long maxCycles) {
    cycles = 0;
    ushort t1;
    PC = pc;
    while (1) {
        byte ir = readByte(PC++);
        switch (ir) {
        case OP_BRK: 
            break;
        case OP_JSR: 
            push((PC+1) & 0xFF);
            push((PC+1) >> 8);
            PC = wordAt(PC);
            break;
        case OP_PHA: push(sys.a); break;
        case OP_RTS:
            if (SP == 0x00) { return cycles; }
            PC = (pop() << 8) | pop();
            PC++;
            break;
        case OP_PLA: REG_A = pop(); break;
        case OP_LDA_IM: REG_A = readByte(PC++); break;
        case OP_NOP: break;
        default: printf("-%d:%d-", PC-1, ir);
            break;
        }
        if (maxCycles && (maxCycles <= cycles)) {
            return cycles;
        }
    }
}

int main()
{
    init();
    printf("6502 Emulator\r\n");
    int h = 0x0600;
    writeByte(h++, OP_LDA_IM);
    writeByte(h++, 0x37);
    writeByte(h++, OP_JSR);
    writeByte(h++, 0);
    writeByte(h++, 7);
    writeByte(h++, OP_LDA_IM);
    writeByte(h++, 0x91);
    writeByte(h++, OP_RTS);
    writeByte(0x0700, OP_RTS);

    run(0x0600, 500);
    printf("\r\ndone. %d cycles", cycles);
}
