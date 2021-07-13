// 6502Emu.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <stdio.h>

typedef unsigned char byte;
typedef unsigned short ushort;

byte mem[65536];
int cycles;
byte sp;
byte a, x, y, lastPage;

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
    for (int i = 0; i < 65536; i++) {
        writeByte(i, i&0xff);
    }
    cycles = lastPage = 0;
    a = x = y = 0;
    sp = 0x00;
}

void push(byte val) { mem[0x0100+(--sp)] = val; }
byte pop() { return mem[0x0100+(sp++)]; }

ushort wordAt(ushort addr) {
    return (readByte(addr) << 8) | readByte(addr + 1);
}

int run(ushort pc, int maxCycles) {
    cycles = 0;
    ushort t1;
    while (1) {
        byte ir = readByte(pc++);
        switch (ir) {
        case 0x00:
            break;
        case 0x01: 
            break;
        case 0x20: 
            push((pc+1) & 0xFF);
            push((pc+1) >> 8);
            pc = wordAt(pc);
            break;
        case 0x60:
            if (sp == 0xFF) { return cycles; }
            pc = (pop() << 8) | pop();
            pc++;
            break;
        default:
            printf("-%d:%d-", pc-1, ir);
            break;
        }
        if (maxCycles && (--maxCycles < 1)) {
            return cycles;
        }
    }
}

int main()
{
    init();
    printf("6502 Emulator\r\n");
    for (ushort i = 0x600; i < 0x700; i++) {
        writeByte(i, i&0xFF);
    }
    for (ushort i = 0x600; i < 0x700; i++) {
        int x = readByte(i);
    }
    run(0x0615, 100);
    printf("\r\ndone. %d cycles", cycles);
}
