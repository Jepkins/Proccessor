#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "proc_flagging.h"
#include "spu_header.h"
#include "processor.h"
#include "stack.h"
#include "dynarr.h"
#include "cpp_preprocessor_logic.h"

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
    proc->code = nullptr;
    proc->regs = (elm_t*)calloc(PROC_REGS_NUMBER, sizeof(*proc->regs));
    assert(proc->regs);
    proc->ram =  (elm_t*)calloc(PROC_RAM_SIZE, sizeof(*proc->ram));
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
            free(proc->code);
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

int main(int argc, char** argv)
{
    StartConfig run_conds;
    proc_setup(argc, argv, &run_conds);

    proc_run(run_conds.input_file);
    printf("done\n");
    return 0;
}


static void proc_run(const char* code_filename)
{
    proc_t* proc = proc_new(sizeof(elm_t));

    size_t code_size = proc_load(code_filename, proc);
    if (code_size == (size_t)-1)
    {
        printf("Failed to load. Break\n");
    }

    while (proc->ip < code_size)
    {
        cmd_code_t cmd = CMD_hlt;
        cmd = proc_execute_next(proc);

        // printf(CMD_CODE_FORMAT " executed\n", cmd);
        if (cmd == CMD_hlt)
            break;
    }
}

static size_t proc_load (const char* code_filename, proc_t* proc)
{
    FILE* istream = fopen(code_filename, "rb");
    if (!istream)
    {
        printf("Could not open file: %s\n", code_filename);
        return (size_t)-1;
    }

    spu_header_t head = {};
    fread(&head, sizeof(head), 1, istream);
    if (ferror(istream))
    {
        printf("Failed to read spu_header\n");
        return (size_t)-1;
    }
    if (!check_header(&head))
    {
        return (size_t)-1;
    }
    size_t code_size = head.code_size;

    proc->code = (char*)calloc(code_size, sizeof(char));
    assert(proc->code && "Allocation error");
    fread(proc->code, sizeof(char), code_size, istream);
    if (ferror(istream))
    {
        printf("Failed to read code\n");
        return (size_t)-1;
    }

    fclose(istream);
    return code_size;
}
static bool check_header (spu_header_t* head)
{
    spu_header_t true_head;
    if (strcmp(head->signature, true_head.signature))
    {
        printf("Failed to check signature: [expected: %s] [got: %s]\n", true_head.signature, head->signature);
        return 0;
    }
    if (head->version != true_head.version)
    {
        printf("Failed to check version: [expected: %lg] [got: %lg]\n", true_head.version, head->version);
        return 0;
    }
    return 1;
}


#define PROC_EXECUTE_CASES_PUSHEND(code, name, ...) __VA_ARGS__ case code: {PRIMITIVE_CAT(proc_execute_, name)(proc); break;}
cmd_code_t proc_execute_next(proc_t* proc)
{
    proc_getfullcmd(proc);

    switch (proc->current_cmd.cmd_code & LAST_BYTE_MASK)
    {
    // Expands to  case 0xff: {proc_execute_unknown(proc); break;}
    //             case 0x00: {proc_execute_hlt(proc); break;}
    //                               ...
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, PROC_EXECUTE_CASES_PUSHEND, PROC_CMD_LIST, )))
    default:
    {
        printf("Unknown command: " CMD_CODE_FORMAT "\n", proc->current_cmd.cmd_code);
        assert(0 && "Unknown command");
    }
    }
    return proc->current_cmd.cmd_code;
}
static void proc_getfullcmd (proc_t* proc)
{
    memcpy(&proc->current_cmd.cmd_code, proc->code + proc->ip, sizeof(proc->current_cmd.cmd_code));
    proc->ip += sizeof(proc->current_cmd.cmd_code);

    proc->current_cmd.arg_ptr = proc_getarg(proc, proc->current_cmd.cmd_code);
}
static elm_t* proc_getarg (proc_t* proc, cmd_code_t cmd)
{
    elm_t value = 0;
    size_t ind = 0;
    bool imd_met = false, reg_met = false;
    if ((imd_met = cmd & IMMEDIATE_MASK))
    {
        memcpy(&value, proc->code + proc->ip, sizeof(value));
        proc->ip += sizeof(value);
    }
    if ((reg_met = cmd & REGISTER_MASK))
    {
        memcpy(&ind, proc->code + proc->ip, sizeof(ind));
        proc->ip += sizeof(ind);
        value += proc->regs[ind];
    }
    if (cmd & RAM_MASK)
    {
        return &proc->ram[value];
    }
    else
    {
        if (!imd_met && reg_met)
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

static void proc_execute_unknown(proc_t* proc)
{
    printf("Unknown command: " CMD_CODE_FORMAT "\n", proc->current_cmd.cmd_code);
    assert(0 && "Unknown command");
}
static void proc_execute_hlt(proc_t* proc)
{
    proc_delete(proc);
    return;
}

// START: JUMPS
static void proc_execute_jmp(proc_t* proc)
{
    proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
static void proc_execute_ja(proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    if (newer_elm > older_elm)
        proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
static void proc_execute_jae(proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    if (newer_elm >= older_elm)
        proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
static void proc_execute_jb(proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    if (newer_elm < older_elm)
        proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
static void proc_execute_jbe(proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    if (newer_elm <= older_elm)
        proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
static void proc_execute_je(proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    if (newer_elm == older_elm)
        proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
static void proc_execute_jne(proc_t* proc)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
    if (newer_elm != older_elm)
        proc->ip = (size_t)*(proc->current_cmd.arg_ptr);
}
// END: JUMPS

static void proc_execute_push(proc_t* proc)
{
    stack_push(proc->stk, proc->current_cmd.arg_ptr);
}
static void proc_execute_pop(proc_t* proc)
{
    stack_pop(proc->stk, proc->current_cmd.arg_ptr);
}

// START: UNARY
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
    stack_pop(proc->stk, &newer_elm);
    stack_pop(proc->stk, &older_elm);
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
// END: UNARY

// START: iostreams
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
    for (size_t i = 0; i < PROC_REGS_NUMBER; i++) fprintf(stderr, "|%2lu: " ELM_T_FORMAT "| ", i, proc->regs[i]);
    fprintf(stderr, "]\n");
}
// END: iostreams
