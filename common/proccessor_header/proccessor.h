#ifndef PROCCESSROR_H
#define PROCCESSROR_H

typedef short cmd_code_t;
typedef int elm_t;
#define ELM_T_FORMAT "%d"
#define CMD_CODE_FORMAT "%4hX"

typedef enum {
    UNKNOWN  = 0xFF,
    CMD_IN   = 0xF1,
    CMD_OUT  = 0xF2,
    CMD_DUMP = 0xF3,






    CMD_HLT  = 0x00,
    CMD_PUSH,
    CMD_POP ,
    CMD_INV ,
    CMD_DUB ,
    CMD_ADD ,
    CMD_SUB ,
    CMD_MUL ,
    CMD_DIV ,
    CMD_SQRT,
    CMD_SIN ,
    CMD_COS
} commands_t;

typedef enum {
    // + REG_ZERO (do not touch)
    REG_AX = 1,
    REG_BX,
    REG_CX,
    REG_DX,
    REG_EX,
    REG_AY,
    REG_BY,
    REG_CY,
    REG_DY,
    REG_EY,
    REG_AZ,
    REG_BZ,
    REG_CZ,
    REG_DZ,
    REG_EZ
} reg_num_t;

#endif // PROCCESSROR_H
