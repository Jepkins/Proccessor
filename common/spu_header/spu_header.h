#ifndef SPU_HEADER_H
#define SPU_HEADER_H

#include <cstdint>
#include "cpp_preprocessor_logic.h"

typedef struct {
    char signature[16] = "Jepkins";
    double version = 3.6;
    size_t code_size = 0;
} spu_header_t;

typedef short cmd_code_t;
#define CMD_CODE_FORMAT "%4hX"
typedef uint32_t elm_t;
#define ELM_T_FORMAT "%u"

static const cmd_code_t SECOND_BYTE_MASK = (cmd_code_t)0xFF00;
static const cmd_code_t LAST_BYTE_MASK   = (cmd_code_t)0x00FF;

static const unsigned char IMMEDIATE_MASK = 0x01;
static const unsigned char REGISTER_MASK  = 0x02;
static const unsigned char RAM_MASK       = 0x04;
static const unsigned char MARK_MASK      = 0x08;

static const unsigned char RVALUE_MASK    = 0xEE;
static const unsigned char LVALUE_MASK    = 0xE4;

#define MAXARGN 256
static const size_t PROC_REGS_NUMBER = 512;
static const size_t PROC_RAM_SIZE = 1'000'000; // 1MB
static const int DRAW_WIDTH = 100;
static const int DRAW_HEIGHT = 100;

#define DEFCMD_(code, name, argn, args) CAT(CMD_, name) = (cmd_code_t) code,
enum command_nums {
    // Expands to [ CMD_unknown = (cmd_code_t) 0xff, CMD_hlt = (cmd_code_t) 0x00, ... ]
    #include "defcmd.h"
};
#undef DEFCMD_

#define DEFREG_(ind, name) CAT(REG_, name) = (size_t) ind,
enum reg_num_t {
    // FIRST MAXARGN (256) regs are reserved!!! (do not touch)
    // Expands to [ REG_AX = (size_t) 256, REG_BX = (size_t) 257, ... ]
    #include "defreg.h"
};
#undef DEFREG_

#endif // SPU_HEADER_H
