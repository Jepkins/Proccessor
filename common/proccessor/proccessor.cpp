#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "proccessor.h"
#include "stack.h"
#include "dynarr.h"

typedef struct {
    stack_t* stk;
    dynarr_t* code;
    size_t ip;
} proc_t;

static size_t proc_load (const char* code_filename, dynarr_t* code);
static cmd_code proc_execute_next(proc_t* proc);

static void proc_execute_push(proc_t* proc);

static void proc_execute_inv (proc_t* proc);
static void proc_execute_dub (proc_t* proc);
static void proc_execute_add (proc_t* proc);
static void proc_execute_sub (proc_t* proc);
static void proc_execute_mul (proc_t* proc);
static void proc_execute_div (proc_t* proc);
static void proc_execute_sqrt(proc_t* proc);
static void proc_execute_sin (proc_t* proc);
static void proc_execute_cos (proc_t* proc);

static void proc_execute_out (proc_t* proc);
static void proc_execute_in  (proc_t* proc);
static void proc_execute_dump  (proc_t* proc);

static proc_t* proc_new (size_t elm_size);
static void proc_ctor (proc_t* proc, size_t elm_size);
static void proc_delete (proc_t* proc);
static void proc_dtor (proc_t* proc);

static proc_t* proc_new (size_t elm_size)
{
    proc_t* proc = (proc_t*)calloc(1, sizeof(*proc));
    proc_ctor(proc, elm_size);
    return proc;
}
static void proc_ctor (proc_t* proc, size_t elm_size)
{
    proc_dtor(proc);
    proc->stk = stack_new(elm_size);
    proc->code = dynarr_new(sizeof(char));
    proc->ip = 0;
}
static void proc_delete (proc_t* proc)
{
    proc_dtor(proc);
    free(proc);
    proc = nullptr;
}
static void proc_dtor (proc_t* proc)
{
    if(proc)
    {
        if (proc->stk)
        {
            stack_delete(proc->stk);
            proc->stk = nullptr;
        }
        if (proc->code)
        {
            dynarr_delete(proc->code);
            proc->code = nullptr;
        }
    }
}

void proc_run(const char* code_filename)
{
    proc_t* proc = proc_new(sizeof(elm_t));

    size_t code_size = proc_load(code_filename, proc->code);
    while (proc->ip < code_size)
    {
        cmd_code cmd = CMD_HLT;
        cmd = proc_execute_next(proc);
        if (cmd == CMD_HLT)
            break;
    }

    proc_delete(proc);
}

static size_t proc_load (const char* code_filename, dynarr_t* code)
{
    FILE* istream = fopen(code_filename, "r");
    if (!istream)
        assert(0 && "could not open file");
    size_t code_size = 0;
    while (!feof(istream))
    {
        cmd_code cmd = 0;
        fscanf(istream, CMD_CODE_FORMAT, &cmd);

        dynarr_push(code, &cmd, sizeof(cmd));
        code_size += sizeof(cmd);
        switch (cmd)
        {
        case CMD_PUSH:
        {
            elm_t arg = 0;
            fscanf(istream, ELM_T_FORMAT, &arg);
            dynarr_push(code, &arg, sizeof(elm_t));
            code_size += sizeof(elm_t);
            break;
        }
        default:
        {
            break;
        }
        }
        if (cmd == CMD_HLT)
            break;
    }

    fclose(istream);
    return code_size;
}

cmd_code proc_execute_next(proc_t* proc)
{
    cmd_code cmd = CMD_HLT;
    dynarr_getdata(proc->code, proc->ip, &cmd, sizeof(cmd));
// proc_execute_dump(proc);
    proc->ip += sizeof(cmd);
    switch (cmd)
    {
    case CMD_PUSH:
    {
        proc_execute_push(proc);
        break;
    }
    case CMD_INV:
    {
        proc_execute_inv(proc);
        break;
    }
    case CMD_DUB:
    {
        proc_execute_dub(proc);
        break;
    }
    case CMD_ADD:
    {
        proc_execute_add(proc);
        break;
    }
    case CMD_SUB:
    {
        proc_execute_sub(proc);
        break;
    }
    case CMD_MUL:
    {
        proc_execute_mul(proc);
        break;
    }
    case CMD_DIV:
    {
        proc_execute_div(proc);
        break;
    }
    case CMD_SQRT:
    {
        proc_execute_sqrt(proc);
        break;
    }
    case CMD_SIN:
    {
        proc_execute_sin(proc);
        break;
    }
    case CMD_COS:
    {
        proc_execute_cos(proc);
        break;
    }
    case CMD_OUT:
    {
        proc_execute_out(proc);
        break;
    }
    case CMD_IN:
    {
        proc_execute_in(proc);
        break;
    }
    case CMD_DUMP:
    {
        proc_execute_dump(proc);
        break;
    }
    case CMD_HLT:
    {
        break;
    }
    default:
    {
        printf("Unknown command: " CMD_CODE_FORMAT "\n", cmd);
        assert(0 && "Unknown command");
    }
    }
    return cmd;
}

static void proc_execute_push(proc_t* proc)
{
    elm_t arg = 0;
    dynarr_getdata(proc->code, proc->ip, &arg, sizeof(arg));
    proc->ip += sizeof(arg);
    stack_push(proc->stk, &arg);
}

static void proc_execute_inv (proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    elm = -elm;
    stack_push(proc->stk, &elm);
}
static void proc_execute_dub (proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    stack_push(proc->stk, &elm);
    stack_push(proc->stk, &elm);
}
static void proc_execute_add (proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &older_elm);
    stack_pop(proc->stk, &newer_elm);
    older_elm += newer_elm;
    stack_push(proc->stk, &older_elm);
}
static void proc_execute_sub (proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    older_elm -= newer_elm;
    stack_push(proc->stk, &older_elm);
}
static void proc_execute_mul (proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    older_elm *= newer_elm;
    stack_push(proc->stk, &older_elm);
}
static void proc_execute_div (proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    older_elm /= newer_elm;
    stack_push(proc->stk, &older_elm);
}
static void proc_execute_sqrt(proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    elm = sqrt(elm);
    stack_push(proc->stk, &elm);
}
static void proc_execute_sin (proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    elm = sin(elm);
    stack_push(proc->stk, &elm);
}
static void proc_execute_cos (proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    elm = cos(elm);
    stack_push(proc->stk, &elm);
}

static void proc_execute_out (proc_t* proc)
{
    elm_t value_to_print;
    stack_pop(proc->stk, &value_to_print);
    printf("out[%lu] = " ELM_T_FORMAT "\n", proc->ip, value_to_print);
}
static void proc_execute_in  (proc_t* proc)
{
    elm_t value_to_scan = 0;
    printf("Enter value: ");
    scanf(ELM_T_FORMAT, &value_to_scan);
    stack_push(proc->stk, &value_to_scan);
}
static void proc_execute_dump(proc_t* proc)
{
    fprintf(stderr, "PROC_DUMP:\n");
    fprintf(stderr, "ip = %lu\n", proc->ip);
    dynarr_dump(proc->code, _POS_);
    stack_dump(proc->stk, _POS_);
}
