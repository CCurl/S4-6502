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
    struct {
        byte c;
        byte z;
        byte i;
        byte d;
        byte x;
        byte b;
        byte v;
        byte n;
    } flags;
} SYS_T;

#define REG_A sys.a
#define REG_X sys.x
#define REG_Y sys.y
#define FLAGS sys.p
#define SP    sys.sp
#define PC    sys.pc
#define STK   0x0100

#define P1 PC++
#define P2 PC += 2
#define P3 PC += 3
#define C(n) cycles += n

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

void push(byte val) { mem[STK+(--SP)] = val; }
byte pop() { return mem[STK+(SP++)]; }

void syncFlags(int p2s) {
    if (p2s) {
        sys.flags.c = (FLAGS & FLAG_C) ? 1 : 0;
        sys.flags.z = (FLAGS & FLAG_Z) ? 1 : 0;
        sys.flags.i = (FLAGS & FLAG_I) ? 1 : 0;
        sys.flags.d = (FLAGS & FLAG_D) ? 1 : 0;
        sys.flags.x = (FLAGS & FLAG_X) ? 1 : 0;
        sys.flags.b = (FLAGS & FLAG_B) ? 1 : 0;
        sys.flags.v = (FLAGS & FLAG_V) ? 1 : 0;
        sys.flags.n = (FLAGS & FLAG_N) ? 1 : 0;
        return;
    }
    sys.p = 0;
    if (sys.flags.c) { sys.p |= FLAG_C; }
    if (sys.flags.z) { sys.p |= FLAG_Z; }
    if (sys.flags.i) { sys.p |= FLAG_I; }
    if (sys.flags.d) { sys.p |= FLAG_D; }
    if (sys.flags.x) { sys.p |= FLAG_X; }
    if (sys.flags.b) { sys.p |= FLAG_B; }
    if (sys.flags.v) { sys.p |= FLAG_V; }
    if (sys.flags.n) { sys.p |= FLAG_N; }
}

void init() {
    for (int i = 0; i <= 0xFFFF; i++) {
        writeByte(i, 0);
    }
    REG_A = REG_X = REG_Y = SP = FLAGS = 0;
    PC = 0;

    lastPage = cycles = 0;
    syncFlags(1);
}

ushort wordAt(ushort addr) {
    return readByte(addr) | (readByte(addr+1) << 8);
}

void doLDA(int addrMode) {
    switch (addrMode) {
    case IMM: REG_A = mem[PC];                 P1; C(2); break;
    case ZP:  REG_A = mem[mem[PC]];            P1; C(3); break;
    case ZPX: REG_A = mem[mem[PC] + REG_X];    P1; C(4); break;
    case ABS: REG_A = mem[wordAt(PC)];         P2; C(4); break;
    case ABX: REG_A = mem[wordAt(PC) + REG_X]; P2; C(4); break;
    case ABY: REG_A = mem[wordAt(PC) + REG_Y]; P2; C(4); break;
    case INX: REG_A = mem[wordAt(PC) + REG_X]; P2; C(4); break;
    case INY: REG_A = mem[wordAt(PC) + REG_Y]; P2; C(4); break;
    default:
        break;
    }
    sys.flags.z = (REG_A) ? 0 : 1;
    sys.flags.n = (REG_A <= 0x7F) ? 0 : 1;
    syncFlags(0);
}

int run(ushort pc, long maxCycles) {
    cycles = 0;
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
        case OP_RTS:
            if (SP == 0x00) { return cycles; }
            PC = (pop() << 8) | pop();
            PC++;
            break;
        case OP_PHA: push(REG_A); break;
        case OP_PLA: REG_A = pop(); break;

        case LDA_IMM: doLDA(IMM); break;
        case LDA_ZP:  doLDA(ZP);  break;
        case LDA_ZPX: doLDA(ZPX); break;
        case LDA_ABS: doLDA(ABS); break;

        case STA_ZP:  mem[mem[PC]] = REG_A;         P1; C(3); break;

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
    writeByte(h++, LDA_IMM);
    writeByte(h++, 0x00);
    writeByte(h++, STA_ZP);
    writeByte(h++, 0x09);
    writeByte(h++, OP_JSR);
    writeByte(h++, 0);
    writeByte(h++, 7);
    writeByte(h++, LDA_ZP);
    writeByte(h++, 0x09);
    writeByte(h++, OP_RTS);
    h = 0x0700;
    writeByte(h++, LDA_IMM);
    writeByte(h++, 0x83);
    writeByte(h++, OP_RTS);

    run(0x0600, 500);
    printf("\r\ndone. %d cycles", cycles);
}
