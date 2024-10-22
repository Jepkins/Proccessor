#ifndef SPU_HEADER_H
#define SPU_HEADER_H

#include "cpp_preprocessor_logic.h"

typedef struct {
    char signature[16] = "Jepkins";
    double version = 2.0;
    size_t code_size = 0;
} spu_header_t;

#define PROC_CMD_LIST \
/*  code  name    */\
/*  |     |       */\
    0xff, unknown,  \
    0x00, hlt,      \
    0x01, push,     \
    0x02, pop,      \
    0xA1, inv,      \
    0xA2, dub,      \
    0xA3, add,      \
    0xA4, sub,      \
    0xA5, mul,      \
    0xA6, div,      \
    0xA7, sqrt,     \
    0xA8, sin,      \
    0xA9, cos,      \
    0xD1, jmp,      \
    0xD2, ja,       \
    0xD3, jae,      \
    0xD4, jb,       \
    0xD5, jbe,      \
    0xD6, je,       \
    0xD7, jne,      \
    0xE1, in,       \
    0xE2, out,      \
    0xE3, dump,     \
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
typedef size_t elm_t;
#define ELM_T_FORMAT "%lu"
#define CMD_CODE_FORMAT "%4hX"

static const cmd_code_t SECOND_BYTE_MASK = (cmd_code_t)0xFF00;
static const cmd_code_t LAST_BYTE_MASK   = (cmd_code_t)0x00FF;
static const cmd_code_t IMMEDIATE_MASK   = (cmd_code_t)0x0100;
static const cmd_code_t REGISTER_MASK    = (cmd_code_t)0x0200;
static const cmd_code_t RAM_MASK         = (cmd_code_t)0x0400;

#define MAX_ARGS_NUMBER 2
static const size_t PROC_REGS_NUMBER = 16;
static const size_t PROC_RAM_SIZE = 1024;

typedef struct {
    const cmd_code_t code;
    const char name[10];
} command_t;

#define NOT_END(t, ...) NOT(CHECK(PRIMITIVE_CAT(NOT_END_, t)))
#define NOT_END_TERMINATOR ~,1,
#define DELETE_FIRST(a, ...) __VA_ARGS__

#define COMMAND_STRUCT_PUSHEND(code, name, ...) __VA_ARGS__ , {(cmd_code_t) code, QUOTE(name)}
static const command_t proc_commands_list[] = {
    // Expands to [ {(cmd_code_t) 0xff, "unknown"}, {(cmd_code_t) 0x00, "hlt"}, ... ]
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, COMMAND_STRUCT_PUSHEND, PROC_CMD_LIST)))
};

#define COMMAND_ENUM_PUSHEND(code, name, ...) __VA_ARGS__ , PRIMITIVE_CAT(CMD_, name) = (cmd_code_t) code
enum commands_nums {
    // Expands to [ CMD_unknown = (cmd_code_t) 0xff, CMD_hlt = (cmd_code_t) 0x00, ... ]
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, COMMAND_ENUM_PUSHEND, PROC_CMD_LIST)))
};

typedef struct {
    const char ind;
    const char name[3];
} registers_t;

#define REGISTER_STRUCT_PUSHEND(ind, name, ...) __VA_ARGS__ , {(char) ind, QUOTE(name)}
static const registers_t proc_registers_list[] = {
    // Expands to [ {1, "AX"}, {2, "BX"}, ... ]
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, REGISTER_STRUCT_PUSHEND, PROC_REGS_LIST)))
};

#define REGISTER_ENUM_PUSHEND(ind, name, ...) __VA_ARGS__ , PRIMITIVE_CAT(REG_, name) = (char) ind
enum reg_num_t {
    // + REG_ZERO (do not touch)
    // Expands to [ REG_AX = (cmd_code_t) 1, REG_BX = (cmd_code_t) 2, ... ]
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, REGISTER_ENUM_PUSHEND, PROC_REGS_LIST)))
};

#endif // SPU_HEADER_H
