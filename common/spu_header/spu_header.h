#ifndef SPU_HEADER_H
#define SPU_HEADER_H

#include "cpp_preprocessor_logic.h"

typedef struct {
    char signature[16] = "Jepkins";
    double version = 2.0;
    size_t code_size = 0;
} spu_header_t;

//                 MRI MI  RI  I
//                 |   |   |   |
// possible args:  0 0 0 0 0 0 0 0
//                   |   |   |   |
//                   MR  M   R   NONE
// I - immediate, R - register, M - RAM
#define PROC_CMD_LIST \
/*  code  name     possible args  */\
/*  |     |        |              */\
    0xff, unknown, 0x00,            \
    0x00, hlt,     0x01,            \
    0x01, push,    0xEE,            \
    0x02, pop,     0xE4,            \
    0xA1, inv,     0x01,            \
    0xA2, dub,     0x01,            \
    0xA3, add,     0x01,            \
    0xA4, sub,     0x01,            \
    0xA5, mul,     0x01,            \
    0xA6, div,     0x01,            \
    0xA7, sqrt,    0x01,            \
    0xA8, sin,     0x01,            \
    0xA9, cos,     0x01,            \
    0xD1, jmp,     0x02,            \
    0xD2, ja,      0x02,            \
    0xD3, jae,     0x02,            \
    0xD4, jb,      0x02,            \
    0xD5, jbe,     0x02,            \
    0xD6, je,      0x02,            \
    0xD7, jne,     0x02,            \
    0xDA, call,    0x02,            \
    0xDB, ret,     0x01,            \
    0xE1, in,      0x01,            \
    0xE2, out,     0x01,            \
    0xE3, dump,    0x01,            \
    TERMINATOR

#define PROC_REGS_LIST \
/*  ind name  */\
/*  |   |     */\
    1,  AX,     \
    2,  BX,     \
    3,  CX,     \
    4,  DX,     \
    5,  EX,     \
    6,  AY,     \
    7,  BY,     \
    8,  CY,     \
    9,  DY,     \
    10, EY,     \
    11, AZ,     \
    12, BZ,     \
    13, CZ,     \
    14, DZ,     \
    15, EZ,     \
    TERMINATOR

typedef short cmd_code_t;
typedef int elm_t;
#define ELM_T_FORMAT "%d"
#define CMD_CODE_FORMAT "%4hX"

static const cmd_code_t SECOND_BYTE_MASK = (cmd_code_t)0xFF00;
static const cmd_code_t LAST_BYTE_MASK   = (cmd_code_t)0x00FF;
static const cmd_code_t IMMEDIATE_MASK   = (cmd_code_t)0x0100;
static const cmd_code_t REGISTER_MASK    = (cmd_code_t)0x0200;
static const cmd_code_t RAM_MASK         = (cmd_code_t)0x0400;
static const cmd_code_t INDEX_MASK         = (cmd_code_t)0x0800;

#define MAX_ARGS_NUMBER 2
static const size_t PROC_REGS_NUMBER = 16;
static const size_t PROC_RAM_SIZE = 1024;


// typedef struct {
//     const cmd_code_t code;
//     const char name[10];
//     const unsigned char possible_args;
// } command_t;
// #define COMMAND_STRUCT_PUSHEND(code, name, args, ...) __VA_ARGS__ , {(cmd_code_t) code, QUOTE(name), (unsigned char) args}
// static const command_t proc_commands_list[] = {
//     // Expands to [ {(cmd_code_t) 0xff, "unknown", (unsigned char) 0x00}, {(cmd_code_t) 0x00, "hlt", (unsigned char) 0x01}, ... ]
//     EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, COMMAND_STRUCT_PUSHEND, PROC_CMD_LIST)))
// };
// #undef COMMAND_STRUCT_PUSHEND

#define COMMAND_ENUM_PUSHEND(code, name, args, ...) __VA_ARGS__ , CAT(CMD_, name) = (cmd_code_t) code
enum commands_nums {
    // Expands to [ CMD_unknown = (cmd_code_t) 0xff, CMD_hlt = (cmd_code_t) 0x00, ... ]
    EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, COMMAND_ENUM_PUSHEND, PROC_CMD_LIST)))
};
#undef COMMAND_ENUM_PUSHEND

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
    // + REG_ZERO (do not touch)
    // Expands to [ REG_AX = (char) 1, REG_BX = (char) 2, ... ]
    EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, REGISTER_ENUM_PUSHEND, PROC_REGS_LIST)))
};
#undef REGISTER_ENUM_PUSHEND

#endif // SPU_HEADER_H
