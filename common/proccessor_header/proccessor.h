#ifndef PROCCESSROR_H
#define PROCCESSROR_H

#include "cpp_preprocessor_logic.h"

#define PROC_CMD_LIST \
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

static const size_t proc_regs_number = 16;
static const size_t proc_ram_size = 1024;

typedef struct {
    const char code;
    const char name[10];
} command_t;

#define NOT_END(t, ...) NOT(CHECK(PRIMITIVE_CAT(NOT_END_, t)))
#define NOT_END_TERMINATOR ~,1,
#define DELETE_FIRST(a, ...) __VA_ARGS__

#define COMMAND_STRUCT_PUSHEND(code, name, ...) __VA_ARGS__ , {(char) code, QUOTE(name)}
static const command_t proc_commands_list[] = {
    // Expands to [ {(char) 0xff, "unknown"}, {(char) 0x00, "hlt"}, ... ]
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, COMMAND_STRUCT_PUSHEND, PROC_CMD_LIST)))
};

#define COMMAND_ENUM_PUSHEND(code, name, ...) __VA_ARGS__ , PRIMITIVE_CAT(CMD_, name) = (char) code
enum commands_nums {
    // Expands to [ CMD_unknown = (char) 0xff, CMD_hlt = (char) 0x00, ... ]
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
    // Expands to [ REG_AX = (char) 1, REG_BX = (char) 2, ... ]
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, REGISTER_ENUM_PUSHEND, PROC_REGS_LIST)))
};

#endif // PROCCESSROR_H
