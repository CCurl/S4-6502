#pragma once

#define OP_BEQ 0xF0
#define OP_BNE 0xD0

#define OP_JMP  0x4C
#define OP_JMPI 0x6C

#define OP_BRK 0x00
#define OP_JSR 0x20
#define OP_PHA 0x48
#define OP_RTS 0x60
#define OP_PLA 0x68
#define OP_NOP 0xEA

#define OP_TAX 0xAA
#define OP_TXA 0x8A
#define OP_TAY 0xA8
#define OP_TYA 0x98
#define OP_TSX 0xBA
#define OP_TXS 0x9A

#define LDA_IMM 0xA9
#define LDA_ZP  0xA5
#define LDA_ZPX 0xB5
#define LDA_ABS 0xAD
#define LDA_ABX 0xBD
#define LDA_ABY 0xB9
#define LDA_INX 0xA1
#define LDA_INY 0xB1

#define LDX_IMM 0xA2
#define LDX_ZP  0xA6
#define LDX_ZPY	0xB6
#define LDX_ABS 0xAE
#define LDX_ABY	0xBE

#define LDY_IMM 0xA0
#define LDY_ZP  0xA4
#define LDY_ZPX	0xB4
#define LDY_ABS 0xAC
#define LDY_ABX	0xBC

#define STA_ZP  0x85

#define IMM 1
#define ZP  2
#define ZPX 3
#define ZPY 4
#define ABS 5
#define ABX 6
#define ABY 7
#define INX 8
#define INY 9


#define FLAG_C (0x01)
#define FLAG_Z (0x02)
#define FLAG_I (0x04)
#define FLAG_D (0x08)
#define FLAG_X (0x10)
#define FLAG_B (0x20)
#define FLAG_V (0x40)
#define FLAG_N (0x80)



