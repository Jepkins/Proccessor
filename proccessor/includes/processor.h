#ifndef PROCESSOR_H
#define PROCESSOR_H

#include "stack.h"
#include "dynarr.h"
#include "spu_header.h"
#include "memcanvas.h"
#include "mytimer.h"

typedef struct {
    elm_t* ptr;
    unsigned char type;
} arg_t;

typedef struct {
    arg_t args[MAXARGN];
    size_t argn;
    cmd_code_t cmd_code;
} full_cmd_t;

typedef struct {
    stack_t* stk;
    char* code;
    size_t code_size;
    size_t ip;
    full_cmd_t current_cmd;
    elm_t* regs;
    elm_t*  ram;
    stack_t* return_ips;
    MemCanvas* cnv;
} proc_t;

static int proc_run();
static size_t proc_load (const char* code_filename, proc_t* proc);
static cmd_code_t proc_execute_next(proc_t* proc);
static void  proc_getfullcmd (proc_t* proc);
static void proc_getargs (proc_t* proc);

static const double IMAGE_SCALE = 4;

#define PROC_EXECUTE_PROTOTYPE_PUSHEND(code, name, argn, args, ...) __VA_ARGS__ static void PRIMITIVE_CAT(proc_execute_,name) (proc_t* proc);
// Expands to [ static void proc_execute_push (proc_t* proc); static void proc_execute_pop (proc_t* proc); ... ]
EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, PROC_EXECUTE_PROTOTYPE_PUSHEND, PROC_CMD_LIST, )))
#undef PROC_EXECUTE_PROTOTYPE_PUSHEND

static bool check_header (spu_header_t* head);
static proc_t* proc_new (size_t elm_size);
static void proc_ctor (proc_t* proc, size_t elm_size);
static void proc_delete (proc_t* proc);
static void proc_dtor (proc_t* proc);

#endif // PROCESSOR_H
