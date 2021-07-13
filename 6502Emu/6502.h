#pragma once

#define OP_BRK 0x00
#define OP_JSR 0x20
#define OP_PHA 0x48
#define OP_RTS 0x60
#define OP_PLA 0x68
#define OP_NOP 0xEA

#define LDA_IMM 0xA9
#define LDA_ZP  0xA5
#define LDA_ZPX 0xB5
#define LDA_ABS 0xAD
#define LDA_ABX 0xBD
#define LDA_ABY 0xB9
#define LDA_INX 0xA1
#define LDA_INY 0xB1

#define STA_ZP  0x85


#define IMM 1
#define ZP  2
#define ZPX 3
#define ABS 4
#define ABX 5
#define ABY 6
#define INX 7
#define INY 8


#define FLAG_C (0x01)
#define FLAG_Z (0x02)
#define FLAG_I (0x04)
#define FLAG_D (0x08)
#define FLAG_X (0x10)
#define FLAG_B (0x20)
#define FLAG_V (0x40)
#define FLAG_N (0x80)
