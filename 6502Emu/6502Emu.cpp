// 6502Emu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>
#include "6502.h"

typedef unsigned char byte;
typedef unsigned short ushort;

byte mem[65536];
long cycles, lastPage, t1, t2, t3;
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

void syncFlags(int fromP) {
    if (fromP) {
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
    FLAGS = 0;
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
    return mem[addr] | (mem[addr+1] << 8);
}

inline byte xB(long x) { return (x & 0xFF); }

void doLDA(int addrMode) {
    switch (addrMode) {
    case IMM: REG_A = mem[PC];                   t2 = 1; t3 = 2; break;
    case ZP:  t1 = mem[PC];                      t2 = 1; t3 = 3; break;
    case ZPX: t1 = xB(mem[PC] + REG_Y);          t2 = 1; t3 = 4; break;
    case ABS: t1 = wordAt(PC);                   t2 = 2; t3 = 4; break;
    case ABX: t1 = wordAt(PC) + REG_X;           t2 = 2; t3 = 4; break;
    case ABY: t1 = wordAt(PC) + REG_Y;           t2 = 2; t3 = 4; break;
    case INX: t1 = wordAt(xB(mem[PC] + REG_X));  t2 = 2; t3 = 6; break;
    case INY: t1 = wordAt(mem[PC]) + REG_Y;      t2 = 2; t3 = 5; break;
    default: break;
    }
    if (addrMode != IMM) { REG_A = mem[t1]; }
    PC += (ushort)t2;
    cycles += t3;
    sys.flags.z = (REG_A) ? 0 : 1;
    sys.flags.n = (REG_A <= 0x7F) ? 0 : 1;
    //syncFlags(0);
}

void doLDX(int addrMode) {
    switch (addrMode) {
    case IMM: REG_X = mem[PC];                   t2 = 1; t3 = 2; break;
    case ZP:  t1 = mem[PC];                      t2 = 1; t3 = 3; break;
    case ZPY: t1 = xB(mem[PC] + REG_Y);          t2 = 1; t3 = 4; break;
    case ABS: t1 = wordAt(PC);                   t2 = 2; t3 = 4; break;
    case ABY: t1 = wordAt(PC) + REG_Y;           t2 = 2; t3 = 4; break;
    default: break;
    }
    if (addrMode != IMM) { REG_X = mem[t1]; }
    PC += (ushort)t2;
    cycles += t3;
    sys.flags.z = (REG_X) ? 0 : 1;
    sys.flags.n = (REG_X <= 0x7F) ? 0 : 1;
    //syncFlags(0);
}

void doLDY(int addrMode) {
    switch (addrMode) {
    case IMM: REG_Y = mem[PC];                   t2 = 1; t3 = 2; break;
    case ZP:  t1 = mem[PC];                      t2 = 1; t3 = 3; break;
    case ZPY: t1 = xB(mem[PC] + REG_X);          t2 = 1; t3 = 4; break;
    case ABS: t1 = wordAt(PC);                   t2 = 2; t3 = 4; break;
    case ABY: t1 = wordAt(PC) + REG_X;           t2 = 2; t3 = 4; break;
    default: break;
    }
    if (addrMode != IMM) { REG_Y = mem[t1]; }
    PC += (ushort)t2;
    cycles += t3;
    sys.flags.z = (REG_Y) ? 0 : 1;
    sys.flags.n = (REG_Y <= 0x7F) ? 0 : 1;
    //syncFlags(0);
}

int run(ushort pc, long maxCycles) {
    cycles = 0;
    PC = pc;
    while (1) {
        byte ir = readByte(PC++);
        printf("-%04x:%02x-", PC-1, ir);
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
        case LDA_ABX: doLDA(ABX); break;
        case LDA_ABY: doLDA(ABY); break;
        case LDA_INX: doLDA(INX); break;
        case LDA_INY: doLDA(INY); break;

        case LDX_IMM: doLDX(IMM); break;
        case LDX_ZP:  doLDX(ZP);  break;
        case LDX_ZPY: doLDX(ZPY); break;
        case LDX_ABS: doLDX(ABS); break;
        case LDX_ABY: doLDX(ABY); break;

        case LDY_IMM: doLDY(IMM); break;
        case LDY_ZP:  doLDY(ZP);  break;
        case LDY_ZPX: doLDY(ZPY); break;
        case LDY_ABS: doLDY(ABS); break;
        case LDY_ABX: doLDY(ABY); break;

        case OP_BEQ: PC += (sys.flags.z) ? mem[PC] : 1;
            cycles += 2;
            break;
        case OP_BNE: PC += (sys.flags.z == 0) ? mem[PC] : 1;
            cycles += 2;
            break;

        case OP_JMP: PC = wordAt(PC);          cycles += 3; break;
        case OP_JMPI: PC = wordAt(wordAt(PC)); cycles += 5; break;

        case STA_ZP:  mem[mem[PC]] = REG_A;         P1; C(3); break;

        case OP_TAX: REG_X = REG_A; cycles += 2; break;
        case OP_TXA: REG_A = REG_X; cycles += 2; break;
        case OP_TAY: REG_Y = REG_A; cycles += 2; break;
        case OP_TYA: REG_A = REG_Y; cycles += 2; break;
        case OP_TSX: SP = REG_X; cycles += 2; break;
        case OP_TXS: REG_X = SP; cycles += 2; break;

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
    for (int i = 0x600; i < h; i++) { if (mem[i]) printf("\n%04x: %02x", i, mem[i]); }
    printf("\r\ndone. %d cycles", cycles);
}
