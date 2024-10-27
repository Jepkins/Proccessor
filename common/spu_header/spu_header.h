#ifndef SPU_HEADER_H
#define SPU_HEADER_H

#include "cpp_preprocessor_logic.h"

typedef struct {
    char signature[16] = "Jepkins";
    double version = 3.1;
    size_t code_size = 0;
} spu_header_t;

//                     MRI MI  RI  I
//                     |   |   |   |
// possible sequence:  0 0 0 0 0 0 0 0    --- lowest byte represents first argument
//                       |   |   |   |
//                       MR  M   R   NONE
// I - immediate, R - register, M - RAM
// Example: EE - rvalues, E4 - lvalues
// Max sequence length = sizeof(int)
//
// seqn - max number of sequences
//
// MORON: possseq = 0x01  <=>  seqn = 0
#define PROC_CMD_LIST \
/*  code  name     seqn   possible sequence  */\
/*  |     |        |      |                  */\
    0xFF, unknown, 0,     0x00,                \
    0xF0, sleep,   1,     0xEE,                \
    0x00, hlt,     0,     0x01,                \
    0x01, push,    10,    0xEE,                \
    0x02, pop,     10,    0xE4,                \
    0x03, mov,     1,     0xEEE4,              \
    0xA1, inv,     0,     0x01,                \
    0xA2, dub,     0,     0x01,                \
    0xA3, add,     0,     0x01,                \
    0xA4, sub,     0,     0x01,                \
    0xA5, mul,     0,     0x01,                \
    0xA6, div,     0,     0x01,                \
    0xA7, sqrt,    0,     0x01,                \
    0xA8, sin,     0,     0x01,                \
    0xA9, cos,     0,     0x01,                \
    0xAA, sqr,     0,     0x01,                \
    0xD1, jmp,     1,     0x02,                \
    0xD2, ja,      1,     0x02,                \
    0xD3, jae,     1,     0x02,                \
    0xD4, jb,      1,     0x02,                \
    0xD5, jbe,     1,     0x02,                \
    0xD6, je,      1,     0x02,                \
    0xD7, jne,     1,     0x02,                \
    0xDA, call,    1,     0x02,                \
    0xDB, ret,     0,     0x01,                \
    0xE0, putcc,   100,   0xEE,                \
    0xE1, in,      0,     0x01,                \
    0xE2, out,     0,     0x01,                \
    0xE3, dump,    0,     0x01,                \
    0xE4, draw,    0,     0x01,                \
    TERMINATOR

#define PROC_REGS_LIST       \
/*first 256 are reserved!!!*/\
/*  ind   name             */\
/*  |     |                */\
    256,  AX,    \
    257,  BX,    \
    258,  CX,    \
    259,  DX,    \
    260,  EX,    \
    261,  AY,    \
    262,  BY,    \
    263,  CY,    \
    264,  DY,    \
    265,  EY,    \
    266,  AZ,    \
    267,  BZ,    \
    268,  CZ,    \
    269,  DZ,    \
    270,  EZ,    \
    271,  RR,    \
    272,  II,    \
    273,  JJ,    \
    274,  AA,    \
    275,  BB,    \
    276,  CC,    \
    277,  DD,    \
    278,  EE,    \
    TERMINATOR

typedef short cmd_code_t;
#define CMD_CODE_FORMAT "%4hX"
typedef int elm_t;
#define ELM_T_FORMAT "%d"

static const cmd_code_t SECOND_BYTE_MASK = (cmd_code_t)0xFF00;
static const cmd_code_t LAST_BYTE_MASK   = (cmd_code_t)0x00FF;
static const unsigned char IMMEDIATE_MASK = (cmd_code_t)0x01;
static const unsigned char REGISTER_MASK  = (cmd_code_t)0x02;
static const unsigned char RAM_MASK       = (cmd_code_t)0x04;
static const unsigned char INDEX_MASK     = (cmd_code_t)0x08;

#define MAXARGN 256
static const size_t PROC_REGS_NUMBER = 512;
static const size_t PROC_RAM_SIZE = 10000;
// static const size_t PROC_VRAM_SIZE = 10000; // FUCK: separate
static const int DRAW_WIDTH = 100;
static const int DRAW_HEIGHT = 100;

// LEGACY
// typedef struct {
//     const cmd_code_t code;
//     const char name[10];
//     const unsigned char max_sequence_n;
//     const unsigned int possible_sequence;
// } command_t;
// #define COMMAND_STRUCT_PUSHEND(code, name, argn, args, ...) __VA_ARGS__ , {(cmd_code_t) code, QUOTE(name), argn, (unsigned char) args}
// static const command_t proc_commands_list[] = {
//     // Expands to [ {(cmd_code_t) 0xff, "unknown", (unsigned char) 0x00}, {(cmd_code_t) 0x00, "hlt", (unsigned char) 0x01}, ... ]
//     EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, COMMAND_STRUCT_PUSHEND, PROC_CMD_LIST)))
// };
// #undef COMMAND_STRUCT_PUSHEND

#define COMMAND_ENUM_PUSHEND(code, name, argn, args, ...) __VA_ARGS__ , CAT(CMD_, name) = (cmd_code_t) code
enum command_nums {
    // Expands to [ CMD_unknown = (cmd_code_t) 0xff, CMD_hlt = (cmd_code_t) 0x00, ... ]
    EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, COMMAND_ENUM_PUSHEND, PROC_CMD_LIST)))
};
#undef COMMAND_ENUM_PUSHEND

// LEGACY
// typedef struct {
//     const char ind;
//     const char name[3];
// } registers_t;
// #define REGISTER_STRUCT_PUSHEND(ind, name, ...) __VA_ARGS__ , {(char) ind, QUOTE(name)}
// static const registers_t proc_registers_list[] = {
//     // Expands to [ {(char) 1, "AX"}, {(char) 2, "BX"}, ... ]
//     EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, REGISTER_STRUCT_PUSHEND, PROC_REGS_LIST)))
// };
// #undef REGISTER_STRUCT_PUSHEND

#define REGISTER_ENUM_PUSHEND(ind, name, ...) __VA_ARGS__ , CAT(REG_, name) = (char) ind
enum reg_num_t {
    // FIRST MAXARGN(256) regs are reserved!!! (do not touch)
    // Expands to [ REG_AX = (char) 1, REG_BX = (char) 2, ... ]
    EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, REGISTER_ENUM_PUSHEND, PROC_REGS_LIST)))
};
#undef REGISTER_ENUM_PUSHEND

#endif // SPU_HEADER_H
