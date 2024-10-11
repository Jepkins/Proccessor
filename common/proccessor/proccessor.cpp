#include <assert.h>
#include <stdio.h>
#include <math.h>
#include "proccessor.h"
#include "stack.h"
#include "queue.h"


#define _PRINT_ELM(elm) printf("elm = %lg\n", elm);
#define _SCAN_ELM(elm) scanf("%lg", elm);

static size_t proc_load (const char* code_filename, queue_t* code);
static int proc_execute_next(queue_t* code, stack_t* stk);

static void proc_execute_push(queue_t* code, stack_t* stk);

static void proc_execute_inv (stack_t* stk);
static void proc_execute_dub (stack_t* stk);
static void proc_execute_add (stack_t* stk);
static void proc_execute_sub (stack_t* stk);
static void proc_execute_mul (stack_t* stk);
static void proc_execute_div (stack_t* stk);
static void proc_execute_sqrt(stack_t* stk);
static void proc_execute_sin (stack_t* stk);
static void proc_execute_cos (stack_t* stk);

static void proc_execute_out (stack_t* stk);
static void proc_execute_in  (stack_t* stk);
static void proc_execute_dump  (queue_t* code, stack_t* stk);

void proc_run(const char* code_filename)
{
    stack_t* stk = stack_new(sizeof(elm_t));
    queue_t* code = queue_new(sizeof(char));

    size_t cmd_number = proc_load(code_filename, code);
    // size_t code_size = queue_curr_size(code);
    for (size_t ip = 0; ip < cmd_number; ip++)
    {
        int cmd = CMD_HLT;
        cmd = proc_execute_next(code, stk);

        if (cmd == CMD_HLT)
            break;
    }

    queue_delete(code);
    stack_delete(stk);
}

static size_t proc_load (const char* code_filename, queue_t* code)
{
    FILE* istream = fopen(code_filename, "r");
    size_t cmd_number = 0;
    while (!feof(istream))
    {
        int cmd = 0;
        fscanf(istream, "%d", &cmd);

        queue_push(code, &cmd, sizeof(cmd));
        cmd_number++;
        switch (cmd)
        {
        case CMD_PUSH:
        {
            elm_t arg = 0;
            fscanf(istream, "%lg", &arg);
            queue_push(code, &arg, sizeof(elm_t));
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
    return cmd_number;
}

int proc_execute_next(queue_t* code, stack_t* stk)
{
    int cmd = CMD_HLT;
    queue_pop(code, &cmd, sizeof(cmd));

    switch (cmd)
    {
    case CMD_PUSH:
    {
        proc_execute_push(code, stk);
        break;
    }
    case CMD_INV:
    {
        proc_execute_inv(stk);
        break;
    }
    case CMD_DUB:
    {
        proc_execute_dub(stk);
        break;
    }
    case CMD_ADD:
    {
        proc_execute_add(stk);
        break;
    }
    case CMD_SUB:
    {
        proc_execute_sub(stk);
        break;
    }
    case CMD_MUL:
    {
        proc_execute_mul(stk);
        break;
    }
    case CMD_DIV:
    {
        proc_execute_div(stk);
        break;
    }
    case CMD_SQRT:
    {
        proc_execute_sqrt(stk);
        break;
    }
    case CMD_SIN:
    {
        proc_execute_sin(stk);
        break;
    }
    case CMD_COS:
    {
        proc_execute_cos(stk);
        break;
    }
    case CMD_OUT:
    {
        proc_execute_out(stk);
        break;
    }
    case CMD_IN:
    {
        proc_execute_in(stk);
        break;
    }
    case CMD_DUMP:
    {
        proc_execute_dump(code, stk);
        break;
    }
    case CMD_HLT:
    {
        break;
    }
    default:
    {
        printf("Unknown command: %d\n", cmd);
        assert(0 && "Unknown command");
    }
    }
    return cmd;
}

static void proc_execute_push(queue_t* code, stack_t* stk)
{
    elm_t arg = 0;
    queue_pop(code, &arg, sizeof(arg));
    stack_push(stk, &arg);
}

static void proc_execute_inv (stack_t* stk)
{
    elm_t elm = 0;
    stack_pop(stk, &elm);
    elm = -elm;
    stack_push(stk, &elm);
}
static void proc_execute_dub (stack_t* stk)
{
    elm_t elm = 0;
    stack_pop(stk, &elm);
    stack_push(stk, &elm);
    stack_push(stk, &elm);
}
static void proc_execute_add (stack_t* stk)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(stk, &older_elm);
    stack_pop(stk, &newer_elm);
    older_elm += newer_elm;
    stack_push(stk, &older_elm);
}
static void proc_execute_sub (stack_t* stk)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(stk, &newer_elm);
    stack_pop(stk, &older_elm);
    older_elm -= newer_elm;
    stack_push(stk, &older_elm);
}
static void proc_execute_mul (stack_t* stk)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(stk, &newer_elm);
    stack_pop(stk, &older_elm);
    older_elm *= newer_elm;
    stack_push(stk, &older_elm);
}
static void proc_execute_div (stack_t* stk)
{
    elm_t newer_elm = 0, older_elm = 0;
    stack_pop(stk, &newer_elm);
    stack_pop(stk, &older_elm);
    older_elm /= newer_elm;
    stack_push(stk, &older_elm);
}
static void proc_execute_sqrt(stack_t* stk)
{
    elm_t elm = 0;
    stack_pop(stk, &elm);
    elm = sqrt(elm);
    stack_push(stk, &elm);
}
static void proc_execute_sin (stack_t* stk)
{
    elm_t elm = 0;
    stack_pop(stk, &elm);
    elm = sin(elm);
    stack_push(stk, &elm);
}
static void proc_execute_cos (stack_t* stk)
{
    elm_t elm = 0;
    stack_pop(stk, &elm);
    elm = cos(elm);
    stack_push(stk, &elm);
}

static void proc_execute_out (stack_t* stk)
{
    elm_t value_to_print;
    stack_pop(stk, &value_to_print);
    _PRINT_ELM(value_to_print);
}
static void proc_execute_in  (stack_t* stk)
{
    elm_t value_to_scan;
    printf("Enter value: ");
    _SCAN_ELM(&value_to_scan);
    stack_push(stk, &value_to_scan);
}
static void proc_execute_dump(queue_t* code, stack_t* stk)
{
    queue_dump(code, _POS_);
    stack_dump(stk, _POS_);
}
