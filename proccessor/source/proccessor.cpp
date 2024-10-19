#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "proc_flagging.h"
#include "proccessor.h"
#include "stack.h"
#include "dynarr.h"

typedef struct {
    stack_t* stk;
    dynarr_t* code;
    size_t ip;
    elm_t* regs;
    elm_t*  ram;
    stack_t* return_ips;
} proc_t;

typedef struct {
    elm_t* arg_ptr;
    cmd_code_t cmd_code;
} full_cmd_t;

static const cmd_code_t SECOND_BYTE_MASK = (cmd_code_t)0xFF00;
static const cmd_code_t LAST_BYTE_MASK   = (cmd_code_t)0x00FF;
static const cmd_code_t IMMEDIATE_MASK   = (cmd_code_t)0x0100;
static const cmd_code_t REGISTER_MASK    = (cmd_code_t)0x0200;
static const cmd_code_t RAM_MASK         = (cmd_code_t)0x0400;


static const size_t regs_number = 16;
static const size_t ram_size = 1024;

static void proc_run(const char* code_filename);
static size_t proc_load (const char* code_filename, dynarr_t* code);
static cmd_code_t proc_execute_next(proc_t* proc);
static void  proc_getfullcmd (proc_t* proc, full_cmd_t* cmd);
static elm_t* proc_getarg (proc_t* proc, cmd_code_t cmd);

static void proc_execute_push (proc_t* proc, elm_t* src);
static void proc_execute_pop (proc_t* proc, elm_t* dst);

static void proc_execute_inv  (proc_t* proc);
static void proc_execute_dub  (proc_t* proc);
static void proc_execute_add  (proc_t* proc);
static void proc_execute_sub  (proc_t* proc);
static void proc_execute_mul  (proc_t* proc);
static void proc_execute_div  (proc_t* proc);
static void proc_execute_sqrt (proc_t* proc);
static void proc_execute_sin  (proc_t* proc);
static void proc_execute_cos  (proc_t* proc);

static void proc_execute_out  (proc_t* proc);
static void proc_execute_in   (proc_t* proc);
static void proc_execute_dump (proc_t* proc);

static proc_t* proc_new (size_t elm_size);
static void proc_ctor (proc_t* proc, size_t elm_size);
static void proc_delete (proc_t* proc);
static void proc_dtor (proc_t* proc);


int main(int argc, char** argv)
{
    StartConfig run_conds;
    proc_setup(argc, argv, &run_conds);

    proc_run(run_conds.input_file);
    return 0;
}


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
    proc->return_ips = stack_new(sizeof(proc->ip));
    proc->code = dynarr_new(sizeof(char));
    proc->regs = (elm_t*)calloc(regs_number, sizeof(*proc->regs));
    assert(proc->regs);
    proc->ram =  (elm_t*)calloc(ram_size, sizeof(*proc->ram));
    assert(proc->ram);
    proc->ip = 0;
}
static void proc_delete (proc_t* proc)
{
    proc_dtor(proc);
    free(proc);
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
        if (proc->return_ips)
        {
            stack_delete(proc->return_ips);
            proc->return_ips = nullptr;
        }
        if (proc->code)
        {
            dynarr_delete(proc->code);
            proc->code = nullptr;
        }
        if (proc->regs)
        {
            free(proc->regs);
            proc->regs = nullptr;
        }
        if (proc->ram)
        {
            free(proc->ram);
            proc->ram = nullptr;
        }
    }
}

static void proc_run(const char* code_filename)
{
    proc_t* proc = proc_new(sizeof(elm_t));

    size_t code_size = proc_load(code_filename, proc->code);

    while (proc->ip < code_size)
    {
        cmd_code_t cmd = CMD_HLT;
        cmd = proc_execute_next(proc);

        printf(CMD_CODE_FORMAT " executed\n", cmd);
        if (cmd == CMD_HLT)
            break;
    }

    proc_delete(proc);
}

static size_t proc_load (const char* code_filename, dynarr_t* code)
{
    FILE* istream = fopen(code_filename, "r");
    assert(istream && "could not open file");

    size_t code_size = 0;
    while (!feof(istream))
    {
        cmd_code_t cmd = 0;
        fscanf(istream, CMD_CODE_FORMAT, &cmd);

        dynarr_push(code, &cmd, sizeof(cmd));
        code_size += sizeof(cmd);
        switch (cmd & LAST_BYTE_MASK)
        {
        case CMD_PUSH:
        case CMD_POP:
        {
            if (cmd & IMMEDIATE_MASK)
            {
                elm_t arg = 0;
                fscanf(istream, ELM_T_FORMAT, &arg);
                dynarr_push(code, &arg, sizeof(arg));
                code_size += sizeof(arg);
            }
            if (cmd & REGISTER_MASK)
            {
                int ind = 0;
                fscanf(istream, ELM_T_FORMAT, &ind);
                dynarr_push(code, &ind, sizeof(ind));
                code_size += sizeof(ind);
            }
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

cmd_code_t proc_execute_next(proc_t* proc)
{
    full_cmd_t cmd = {};
    proc_getfullcmd(proc, &cmd);

    switch (cmd.cmd_code & LAST_BYTE_MASK)
    {
    case CMD_PUSH:
    {
        proc_execute_push(proc, cmd.arg_ptr);
        break;
    }
    case CMD_POP:
    {
        proc_execute_pop(proc, cmd.arg_ptr);
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
        printf("Unknown command: " CMD_CODE_FORMAT "\n", cmd.cmd_code);
        assert(0 && "Unknown command");
    }
    }
    return cmd.cmd_code;
}
static void proc_getfullcmd (proc_t* proc, full_cmd_t* cmd)
{
    dynarr_getdata(proc->code, proc->ip, &cmd->cmd_code, sizeof(cmd->cmd_code));
    proc->ip += sizeof(cmd->cmd_code);

    cmd->arg_ptr = proc_getarg(proc, cmd->cmd_code);
}
static elm_t* proc_getarg (proc_t* proc, cmd_code_t cmd)
{
    elm_t value = 0;
    int ind = 0;
    bool im = false, rm = false;
    if ((im = cmd & IMMEDIATE_MASK))
    {
        dynarr_getdata(proc->code, proc->ip, &value, sizeof(value));
        proc->ip += sizeof(value);
    }
    if ((rm = cmd & REGISTER_MASK))
    {
        dynarr_getdata(proc->code, proc->ip, &ind, sizeof(ind));
        proc->ip += sizeof(ind);
        value += proc->regs[ind];
    }
    if (cmd & RAM_MASK)
    {
        return &proc->ram[value];
    }
    else
    {
        if (!im && rm)
        {
            return &proc->regs[ind];
        }
        else
        {
            proc->regs[0] = value;
            return &proc->regs[0];
        }
    }
}


static void proc_execute_push(proc_t* proc, elm_t* src)
{
    stack_push(proc->stk, src);
}
static void proc_execute_pop(proc_t* proc, elm_t* dst)
{
    stack_pop(proc->stk, dst);
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
    elm = (elm_t)sqrt((double)elm);
    stack_push(proc->stk, &elm);
}
static void proc_execute_sin (proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    elm = (elm_t)sin((double)elm);
    stack_push(proc->stk, &elm);
}
static void proc_execute_cos (proc_t* proc)
{
    elm_t elm = 0;
    stack_pop(proc->stk, &elm);
    elm = (elm_t)cos((double)elm);
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
    //dynarr_dump(proc->code, _POS_);
    stack_dump(proc->stk, _POS_);
    fprintf(stderr, "regs: [ ");
    for (size_t i = 0; i < regs_number; i++) fprintf(stderr, "|%2lu: " ELM_T_FORMAT "| ", i, proc->regs[i]);
    fprintf(stderr, "]\n");
}
