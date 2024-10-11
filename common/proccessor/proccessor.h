#ifndef PROCCESSROR_H
#define PROCCESSROR_H

typedef double elm_t;

typedef enum {
    UNKNOWN = -100,
    CMD_IN =   -3,
    CMD_OUT =  -2,
    CMD_DUMP = -1,
    CMD_HLT =   0,
    CMD_PUSH =  1,
    CMD_INV  =  2,
    CMD_DUB =   3,
    CMD_ADD =   4,
    CMD_SUB =   5,
    CMD_MUL =   6,
    CMD_DIV =   7,
    CMD_SQRT =  8,
    CMD_SIN =   9,
    CMD_COS =   10
} commands_t;

void proc_run(const char* code_filename);

#endif // PROCCESSROR_H
