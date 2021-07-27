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



void op_BRK(int MODE) {}
void op_ORA(int MODE) {}
void op_ASL(int MODE) {}
void op_PHP(int MODE) {}
void op_BPL(int MODE) {}
void op_CLC(int MODE) {}
void op_JSR(int MODE) {}
void op_AND(int MODE) {}
void op_BIT(int MODE) {}
void op_ROL(int MODE) {}
void op_PLP(int MODE) {}
void op_BMI(int MODE) {}
void op_SEC(int MODE) {}
void op_RTI(int MODE) {}
void op_EOR(int MODE) {}
void op_LSR(int MODE) {}
void op_CLI(int MODE) {}
void op_PHA(int MODE) {}
void op_JMP(int MODE) {}
void op_BVC(int MODE) {}
void op_RTS(int MODE) {}
void op_ADC(int MODE) {}
void op_ROR(int MODE) {}
void op_PLA(int MODE) {}
void op_BVS(int MODE) {}
void op_SEI(int MODE) {}
void op_STA(int MODE) {}
void op_STY(int MODE) {}
void op_STX(int MODE) {}
void op_DEY(int MODE) {}
void op_TXA(int MODE) {}
void op_BCC(int MODE) {}
void op_TYA(int MODE) {}
void op_TXS(int MODE) {}
void op_LDY(int MODE) {}
void op_LDA(int MODE) {}
void op_LDX(int MODE) {}
void op_TAY(int MODE) {}
void op_TAX(int MODE) {}
void op_BCS(int MODE) {}
void op_CLV(int MODE) {}
void op_TSX(int MODE) {}
void op_CPY(int MODE) {}
void op_CMP(int MODE) {}
void op_DEC(int MODE) {}
void op_INY(int MODE) {}
void op_DEX(int MODE) {}
void op_CLD(int MODE) {}
void op_CPX(int MODE) {}
void op_SBC(int MODE) {}
void op_INC(int MODE) {}
void op_INX(int MODE) {}
void op_NOP(int MODE) {}
void op_BEQ(int MODE) {}
void op_SED(int MODE) {}

void xxx(int ir)
{
    int MODE = 0;
	switch (ir) {
        case 0: op_BRK(MODE); break;
        case 1: op_ORA(MODE); break;
        //	case 2: op_cop(MODE); break;
        //	case 3: op_ora(MODE); break;
        //	case 4: op_Tsb(MODE); break;
        case 5: op_ORA(MODE); break;
        case 6: op_ASL(MODE); break;
        //	case 7: op_ora(MODE); break;
        case 8: op_PHP(MODE); break;
        case 9: op_ORA(MODE); break;
        case 10: op_ASL(MODE); break;
        //	case 11: op_phd(MODE); break;
        //	case 12: op_Tsb(MODE); break;
        case 13: op_ORA(MODE); break;
        case 14: op_ASL(MODE); break;
        //	case 15: op_ora(MODE); break;
        case 16: op_BPL(MODE); break;
        case 17: op_ORA(MODE); break;
        //	case 18: op_Ora(MODE); break;
        //	case 19: op_ora(MODE); break;
        //	case 20: op_Trb(MODE); break;
        case 21: op_ORA(MODE); break;
        case 22: op_ASL(MODE); break;
        //	case 23: op_ora(MODE); break;
        case 24: op_CLC(MODE); break;
        case 25: op_ORA(MODE); break;
        //	case 26: op_Inc(MODE); break;
        //	case 27: op_tcs(MODE); break;
        //	case 28: op_Trb(MODE); break;
        case 29: op_ORA(MODE); break;
        case 30: op_ASL(MODE); break;
        //	case 31: op_ora(MODE); break;
        case 32: op_JSR(MODE); break;
        case 33: op_AND(MODE); break;
        //	case 34: op_jsl(MODE); break;
        //	case 35: op_and(MODE); break;
        case 36: op_BIT(MODE); break;
        case 37: op_AND(MODE); break;
        case 38: op_ROL(MODE); break;
        //	case 39: op_and(MODE); break;
        case 40: op_PLP(MODE); break;
        case 41: op_AND(MODE); break;
        case 42: op_ROL(MODE); break;
        //	case 43: op_pld(MODE); break;
        case 44: op_BIT(MODE); break;
        case 45: op_AND(MODE); break;
        case 46: op_ROL(MODE); break;
        //	case 47: op_and(MODE); break;
        case 48: op_BMI(MODE); break;
        case 49: op_AND(MODE); break;
        //	case 50: op_And(MODE); break;
        //	case 51: op_and(MODE); break;
        //	case 52: op_Bit(MODE); break;
        case 53: op_AND(MODE); break;
        case 54: op_ROL(MODE); break;
        //	case 55: op_and(MODE); break;
        case 56: op_SEC(MODE); break;
        case 57: op_AND(MODE); break;
        //	case 58: op_Dec(MODE); break;
        //	case 59: op_tsc(MODE); break;
        //	case 60: op_Bit(MODE); break;
        case 61: op_AND(MODE); break;
        case 62: op_ROL(MODE); break;
        //	case 63: op_and(MODE); break;
        case 64: op_RTI(MODE); break;
        case 65: op_EOR(MODE); break;
        //	case 66: op_wdm(MODE); break;
        //	case 67: op_eor(MODE); break;
        //	case 68: op_mvp(MODE); break;
        case 69: op_EOR(MODE); break;
        case 70: op_LSR(MODE); break;
        //	case 71: op_eor(MODE); break;
        case 72: op_PHA(MODE); break;
        case 73: op_EOR(MODE); break;
        case 74: op_LSR(MODE); break;
        //	case 75: op_phk(MODE); break;
        case 76: op_JMP(MODE); break;
        case 77: op_EOR(MODE); break;
        case 78: op_LSR(MODE); break;
        //	case 79: op_eor(MODE); break;
        case 80: op_BVC(MODE); break;
        case 81: op_EOR(MODE); break;
        //case 82: op_Eor(MODE); break;
        //	case 83: op_eor(MODE); break;
        //	case 84: op_mvn(MODE); break;
        case 85: op_EOR(MODE); break;
        case 86: op_LSR(MODE); break;
        //	case 87: op_eor(MODE); break;
        case 88: op_CLI(MODE); break;
        case 89: op_EOR(MODE); break;
        //	case 90: op_Phy(MODE); break;
        //	case 91: op_tcd(MODE); break;
        //	case 92: op_jmp(MODE); break;
        case 93: op_EOR(MODE); break;
        case 94: op_LSR(MODE); break;
        //	case 95: op_eor(MODE); break;
        case 96: op_RTS(MODE); break;
        case 97: op_ADC(MODE); break;
        //	case 98: op_per(MODE); break;
        //	case 99: op_adc(MODE); break;
        //	case 100: op_Stz(MODE); break;
        case 101: op_ADC(MODE); break;
        case 102: op_ROR(MODE); break;
        //	case 103: op_adc(MODE); break;
        case 104: op_PLA(MODE); break;
        case 105: op_ADC(MODE); break;
        case 106: op_ROR(MODE); break;
        //	case 107: op_rtl(MODE); break;
        case 108: op_JMP(MODE); break;
        case 109: op_ADC(MODE); break;
        case 110: op_ROR(MODE); break;
        //	case 111: op_adc(MODE); break;
        case 112: op_BVS(MODE); break;
        case 113: op_ADC(MODE); break;
        //	case 114: op_Adc(MODE); break;
        //	case 115: op_adc(MODE); break;
        //	case 116: op_Stz(MODE); break;
        case 117: op_ADC(MODE); break;
        case 118: op_ROR(MODE); break;
        //	case 119: op_adc(MODE); break;
        case 120: op_SEI(MODE); break;
        case 121: op_ADC(MODE); break;
        //	case 122: op_Ply(MODE); break;
        //	case 123: op_tdc(MODE); break;
        //	case 124: op_Jmp(MODE); break;
        case 125: op_ADC(MODE); break;
        case 126: op_ROR(MODE); break;
        //	case 127: op_adc(MODE); break;
        //	case 128: op_Bra(MODE); break;
        case 129: op_STA(MODE); break;
        //	case 130: op_brl(MODE); break;
        //	case 131: op_sta(MODE); break;
        case 132: op_STY(MODE); break;
        case 133: op_STA(MODE); break;
        case 134: op_STX(MODE); break;
        //	case 135: op_sta(MODE); break;
        case 136: op_DEY(MODE); break;
        //	case 137: op_Bit(MODE); break;
        case 138: op_TXA(MODE); break;
        //	case 139: op_phb(MODE); break;
        case 140: op_STY(MODE); break;
        case 141: op_STA(MODE); break;
        case 142: op_STX(MODE); break;
        //	case 143: op_sta(MODE); break;
        case 144: op_BCC(MODE); break;
        case 145: op_STA(MODE); break;
        //	case 146: op_Sta(MODE); break;
        //	case 147: op_sta(MODE); break;
        case 148: op_STY(MODE); break;
        case 149: op_STA(MODE); break;
        case 150: op_STX(MODE); break;
        //	case 151: op_sta(MODE); break;
        case 152: op_TYA(MODE); break;
        case 153: op_STA(MODE); break;
        case 154: op_TXS(MODE); break;
        //	case 155: op_txy(MODE); break;
        //	case 156: op_Stz(MODE); break;
        case 157: op_STA(MODE); break;
        //	case 158: op_Stz(MODE); break;
        //	case 159: op_sta(MODE); break;
        case 160: op_LDY(MODE); break;
        case 161: op_LDA(MODE); break;
        case 162: op_LDX(MODE); break;
        //	case 163: op_lda(MODE); break;
        case 164: op_LDY(MODE); break;
        case 165: op_LDA(MODE); break;
        case 166: op_LDX(MODE); break;
        //	case 167: op_lda(MODE); break;
        case 168: op_TAY(MODE); break;
        case 169: op_LDA(MODE); break;
        case 170: op_TAX(MODE); break;
        //	case 171: op_plb(MODE); break;
        case 172: op_LDY(MODE); break;
        case 173: op_LDA(MODE); break;
        case 174: op_LDX(MODE); break;
        //	case 175: op_lda(MODE); break;
        case 176: op_BCS(MODE); break;
        case 177: op_LDA(MODE); break;
        //	case 178: op_Lda(MODE); break;
        //	case 179: op_lda(MODE); break;
        case 180: op_LDY(MODE); break;
        case 181: op_LDA(MODE); break;
        case 182: op_LDX(MODE); break;
        //	case 183: op_lda(MODE); break;
        case 184: op_CLV(MODE); break;
        case 185: op_LDA(MODE); break;
        case 186: op_TSX(MODE); break;
        //	case 187: op_tyx(MODE); break;
        case 188: op_LDY(MODE); break;
        case 189: op_LDA(MODE); break;
        case 190: op_LDX(MODE); break;
        //	case 191: op_lda(MODE); break;
        case 192: op_CPY(MODE); break;
        case 193: op_CMP(MODE); break;
        //	case 194: op_rep(MODE); break;
        //	case 195: op_cmp(MODE); break;
        case 196: op_CPY(MODE); break;
        case 197: op_CMP(MODE); break;
        case 198: op_DEC(MODE); break;
        //	case 199: op_cmp(MODE); break;
        case 200: op_INY(MODE); break;
        case 201: op_CMP(MODE); break;
        case 202: op_DEX(MODE); break;
        //	case 203: op_wai(MODE); break;
        case 204: op_CPY(MODE); break;
        case 205: op_CMP(MODE); break;
        case 206: op_DEC(MODE); break;
        //	case 207: op_cmp(MODE); break;
        //	case 208: op_BNE(MODE); break;
        case 209: op_CMP(MODE); break;
        //	case 210: op_Cmp(MODE); break;
        //	case 211: op_cmp(MODE); break;
        //	case 212: op_pei(MODE); break;
        case 213: op_CMP(MODE); break;
        case 214: op_DEC(MODE); break;
        //	case 215: op_cmp(MODE); break;
        case 216: op_CLD(MODE); break;
        case 217: op_CMP(MODE); break;
        //	case 218: op_Phx(MODE); break;
        //	case 219: op_stp(MODE); break;
        //	case 220: op_jml(MODE); break;
        case 221: op_CMP(MODE); break;
        case 222: op_DEC(MODE); break;
        //	case 223: op_cmp(MODE); break;
        case 224: op_CPX(MODE); break;
        case 225: op_SBC(MODE); break;
        //	case 226: op_sep(MODE); break;
        //	case 227: op_sbc(MODE); break;
        case 228: op_CPX(MODE); break;
        case 229: op_SBC(MODE); break;
        case 230: op_INC(MODE); break;
        //	case 231: op_sbc(MODE); break;
        case 232: op_INX(MODE); break;
        case 233: op_SBC(MODE); break;
        case 234: op_NOP(MODE); break;
        //	case 235: op_xba(MODE); break;
        case 236: op_CPX(MODE); break;
        case 237: op_SBC(MODE); break;
        case 238: op_INC(MODE); break;
        //	case 239: op_sbc(MODE); break;
        case 240: op_BEQ(MODE); break;
        case 241: op_SBC(MODE); break;
        //	case 242: op_Sbc(MODE); break;
        //	case 243: op_sbc(MODE); break;
        //	case 244: op_pea(MODE); break;
        case 245: op_SBC(MODE); break;
        case 246: op_INC(MODE); break;
        //	case 247: op_sbc(MODE); break;
        case 248: op_SED(MODE); break;
        case 249: op_SBC(MODE); break;
        //	case 250: op_Plx(MODE); break;
        //	case 251: op_xce(MODE); break;
        //	case 252: op_jsr(MODE); break;
        case 253: op_SBC(MODE); break;
        case 254: op_INC(MODE); break;
        //	case 255: op_sbc(MODE); break;
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
