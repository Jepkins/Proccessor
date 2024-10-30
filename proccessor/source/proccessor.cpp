#include <assert.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <chrono>
#include <thread>
#include "proc_flagging.h"
#include "spu_header.h"
#include "processor.h"
#include "stack.h"
#include "dynarr.h"
#include "cpp_preprocessor_logic.h"
#include "memcanvas.h"

static StartConfig run_conds;

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
    timer_cl t;
    t.start();
    proc_setup(argc, argv, &run_conds);

    if (proc_run() != 0)
        return 1;
    printf("done\n");
    printf("TIME = %lu mks\n", t.mictime());
    return 0;
}


static int proc_run()
{
    proc_t* proc = proc_new(sizeof(elm_t));
    size_t code_size = proc_load(run_conds.input_file, proc);
    if (code_size == (size_t)-1)
    {
        printf("Failed to load. Break\n");
        return 1;
    }
    proc->code_size = code_size;
    while (!proc->stop)
    {
        if (run_conds.do_video)
        {
            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                if (event.type == SDL_QUIT)
                {
                    run_conds.do_video = false;
                    proc->cnv->Quit();
                }
            }
        }

        cmd_code_t cmd = CMD_hlt;
        cmd = proc_execute_next(proc);

        if (cmd == CMD_hlt)
            break;
    }

    if (run_conds.do_video)
    {
        proc->cnv->Quit();
    }
    proc_delete (proc);
    return 0;
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

    if (run_conds.do_video)
    {
        if (PROC_RAM_SIZE * sizeof(elm_t) < DRAW_WIDTH * DRAW_HEIGHT * sizeof(pix_t))
        {
            printf("Not enough ram to draw!!!\n");
            run_conds.do_video = false;
            return (size_t)-1;
        }
        else
        {
            proc->cnv = new MemCanvas;
            int init_ret = proc->cnv->Init((pix_t*)proc->ram, DRAW_WIDTH, DRAW_HEIGHT, "RAM[]");
            if (init_ret != 0)
            {
                printf("Failed to initialize canvas!\n");
                run_conds.do_video = false;
                return (size_t)-1;
            }
        }
    }

    proc->timer.start();
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


#define PROC_EXECUTE_CASES_PUSHEND(code, name, argn, args, ...) __VA_ARGS__ case code: {CAT(proc_execute_, name)(proc); break;}
cmd_code_t proc_execute_next(proc_t* proc)
{
    // printf("Entered execute_next\n");
    proc_getfullcmd(proc);
    // printf("Got instruction: code = %hX, argn = %lu\n", proc->current_cmd.cmd_code, proc->current_cmd.argn);
    switch (proc->current_cmd.cmd_code & LAST_BYTE_MASK)
    {
    // Expands to  case 0xff: {proc_execute_unknown(proc); break;}
    //             case 0x00: {proc_execute_hlt(proc); break;}
    //                               ...
    EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, PROC_EXECUTE_CASES_PUSHEND, PROC_CMD_LIST, )))
    default:
    {
        printf("Unknown command: " CMD_CODE_FORMAT "\n", proc->current_cmd.cmd_code);
        assert(0 && "Unknown command");
    }
    }
    return proc->current_cmd.cmd_code;
}
#undef PROC_EXECUTE_CASES_PUSHEND

#define CMD_CODE_ proc->current_cmd.cmd_code
#define ARG_PTR_(n) proc->current_cmd.args[n].ptr
#define ARG_TYPE_(n) proc->current_cmd.args[n].type
#define ARGN_ proc->current_cmd.argn
static void proc_getfullcmd (proc_t* proc)
{
    memcpy(&CMD_CODE_, proc->code + proc->ip, sizeof(CMD_CODE_));
    proc->ip += sizeof(CMD_CODE_);
    ARGN_ = (unsigned char)((CMD_CODE_ & SECOND_BYTE_MASK) >> 8);
    // printf("code = %hX, argn = %lu\n", CMD_CODE_, ARGN_);
    proc_getargs(proc);
}
static void proc_getargs (proc_t* proc)
{
    for (size_t argi = 0; argi < ARGN_; argi++)
    {
        elm_t value = 0;
        size_t ind = 0;
        unsigned char type = 0;
        // printf("getarg: ip = %lu\n", proc->ip);
        memcpy(&type, proc->code + proc->ip, sizeof(type));
        proc->ip += sizeof(type);
        ARG_TYPE_(argi) = type;
        if (type & MARK_MASK)
        {
            memcpy(&ind, proc->code + proc->ip, sizeof(ind));
            proc->ip += sizeof(ind);
            proc->regs[argi] = (elm_t)ind;
            ARG_PTR_(argi) = &proc->regs[argi];
            continue;
        }
        bool imd_met = false, reg_met = false;
        if ((imd_met = type & IMMEDIATE_MASK))
        {
            memcpy(&value, proc->code + proc->ip, sizeof(value));
            proc->ip += sizeof(value);
        }
        if ((reg_met = type & REGISTER_MASK))
        {
            memcpy(&ind, proc->code + proc->ip, sizeof(ind));
            proc->ip += sizeof(ind);
            value += proc->regs[ind];
        }
        if (type & RAM_MASK)
        {
            assert((size_t)round(value) < PROC_RAM_SIZE && "RAM: out of boundaries");
            ARG_PTR_(argi) = &proc->ram[(size_t)round(value)];
        }
        else
        {
            if (!imd_met && reg_met)
            {
                ARG_PTR_(argi) = &proc->regs[ind];
            }
            else
            {
                proc->regs[argi] = value;
                ARG_PTR_(argi) = &proc->regs[argi];
            }
        }
    }
}
#undef CMD_CODE_
#undef ARG_PTR_
#undef ARG_TYPE_
#undef ARGN_
/*================================================================================================================================*/

#define ARG_(n) *(proc->current_cmd.args[n].ptr)
#define ARGN_ (proc->current_cmd.argn)
#define CMD_  (proc->current_cmd.cmd_code)
#define POP_(elm)  elm_t elm = 0; stack_pop(proc->stk, &elm);
#define PUSH_(elm) stack_push(proc->stk, &elm);


static void proc_execute_unknown(proc_t* proc)
{
    printf("Unknown command: " CMD_CODE_FORMAT "\n", CMD_);
    assert(0 && "Unknown command");
}
static void proc_execute_hlt(proc_t* proc)
{
    proc->stop = true;
    return;
}

// START: JUMPS
#define POP_POP(elm_new, elm_old)       \
    POP_(elm_new)                       \
    POP_(elm_old)

static void proc_execute_jmp(proc_t* proc)
{
    proc->ip = (size_t)ARG_(0);
}
static void proc_execute_ja(proc_t* proc)
{
    POP_POP(elm_new, elm_old)
    if (elm_new > elm_old)
        proc->ip = (size_t)ARG_(0);
}
static void proc_execute_jae(proc_t* proc)
{
    POP_POP(elm_new, elm_old)
    if (elm_new >= elm_old)
        proc->ip = (size_t)ARG_(0);
}
static void proc_execute_jb(proc_t* proc)
{
    POP_POP(elm_new, elm_old)
    if (elm_new < elm_old)
        proc->ip = (size_t)ARG_(0);
}
static void proc_execute_jbe(proc_t* proc)
{
    POP_POP(elm_new, elm_old)
    if (elm_new <= elm_old)
        proc->ip = (size_t)ARG_(0);
}
static void proc_execute_je(proc_t* proc)
{
    POP_POP(elm_new, elm_old)
    if (elm_new == elm_old)
        proc->ip = (size_t)ARG_(0);
}
static void proc_execute_jne(proc_t* proc)
{
    POP_POP(elm_new, elm_old)
    if (elm_new != elm_old)
        proc->ip = (size_t)ARG_(0);
}

#undef POP_POP
// END: JUMPS

// START: FUNCS
static void proc_execute_call(proc_t* proc)
{
    size_t ip_next = proc->ip;
    stack_push(proc->return_ips, &ip_next);
    proc->ip = (size_t)ARG_(0);
}
static void proc_execute_ret(proc_t* proc)
{
    size_t ret_ip = 0;
    stack_pop(proc->return_ips, &ret_ip);
    proc->ip = ret_ip;
}
// END: FUNCS

// START: MEMOPERS
static void proc_execute_push(proc_t* proc)
{
    for (size_t i = 0; i < ARGN_; i++)
    {
        PUSH_(ARG_(i))
    }
}
static void proc_execute_pop(proc_t* proc)
{
    POP_(elm)
    for (size_t i = 0; i < ARGN_; i++)
    {
        memcpy(&ARG_(i), &elm, sizeof(elm));
    }
}
static void proc_execute_mov(proc_t* proc)
{
    memcpy(&ARG_(0), &ARG_(1), sizeof(ARG_(0)));
}
static void proc_execute_mst(proc_t* proc)
{
    // for (size_t i = 0; i < (size_t)ARG_(2); i++)
    // {
    //     *(&ARG_(0) + i) = ARG_(1);
    // }
    std::fill(&ARG_(0), (&ARG_(0) + (size_t)ARG_(2)), ARG_(1));
}
// END: MEMOPERS

// START: UNARY

#define POP_PUSH(elm, oper)             \
    POP_(elm)                           \
    elm_t res = oper;                   \
    PUSH_(res)

static void proc_execute_inv (proc_t* proc)
{
    POP_PUSH(elm, -elm)
}
static void proc_execute_dub (proc_t* proc)
{
    POP_(elm)
    PUSH_(elm)
    PUSH_(elm)
}
static void proc_execute_sqrt(proc_t* proc)
{
    POP_PUSH(elm, (elm_t)sqrt((double)elm))
}
static void proc_execute_sqr(proc_t* proc)
{
    POP_PUSH(elm, elm*elm)
}
static void proc_execute_sin (proc_t* proc)
{
    POP_PUSH(elm, (elm_t)sin((double)elm))
}
static void proc_execute_cos (proc_t* proc)
{
    POP_PUSH(elm, (elm_t)cos((double)elm))
}

#undef POP_PUSH

// END: UNARY

// START: BIARG

#define POP_POP_PUSH(elm_new, elm_old, oper)    \
    POP_(elm_new)                               \
    POP_(elm_old)                               \
    elm_t res = oper;                           \
    PUSH_(res)

static void proc_execute_add (proc_t* proc)
{
    POP_POP_PUSH(elm_new, elm_old, elm_old + elm_new)
}
static void proc_execute_sub (proc_t* proc)
{
    POP_POP_PUSH(elm_new, elm_old, elm_old - elm_new)
}
static void proc_execute_mul (proc_t* proc)
{
    POP_POP_PUSH(elm_new, elm_old, elm_old * elm_new)
}
static void proc_execute_div (proc_t* proc)
{
    POP_POP_PUSH(elm_new, elm_old, elm_old / elm_new)
}

#undef POP_POP_PUSH

// END: BIARG

// START: iostreams
static void proc_execute_putcc (proc_t* proc)
{
    putc((int)ARG_(0), stdout);
}
static void proc_execute_out (proc_t* proc)
{
    POP_(value_to_print)
    printf(ELM_T_FORMAT "\n", value_to_print);
}
static void proc_execute_in  (proc_t* proc)
{
    elm_t value_to_scan = 0;
    scanf(ELM_T_FORMAT, &value_to_scan);
    PUSH_(value_to_scan);
}
static void proc_execute_dump(proc_t* proc)
{
    fprintf(stderr, "PROC_DUMP:\n");
    fprintf(stderr, "ip = %lu\n", proc->ip);
    fprintf(stderr, "code: [ ");
    for (size_t i = 0; i < proc->code_size; i++) fprintf(stderr, "|%2lu: %2X| ", i, proc->code[i]);
    fprintf(stderr, "]\n");
    fprintf(stderr, "ram: [ ");
    for (size_t i = 0; i < PROC_RAM_SIZE; i++) fprintf(stderr, "|%2lu: " ELM_T_FORMAT "| ", i, proc->ram[i]);
    fprintf(stderr, "]\n");
    stack_dump(proc->stk, _POS_);
    fprintf(stderr, "regs: [ ");
    for (size_t i = 0; i < PROC_REGS_NUMBER; i++) fprintf(stderr, "|%2lu: " ELM_T_FORMAT "| ", i, proc->regs[i]);
    fprintf(stderr, "]\n");
}
// END: iostreams

// START: MISC
static void proc_execute_draw  (proc_t* proc)
{
    if (!run_conds.do_video)
    {
        printf("Trying to draw with video disabled! Stop.\n");
        proc->stop = true;
        return;
    }
    proc->cnv->Update();
}
static void proc_execute_sleep  (proc_t* proc)
{
    std::this_thread::sleep_for(std::chrono::microseconds((size_t) ARG_(0)));
}
static void proc_execute_slpdif  (proc_t* proc)
{
    size_t time = proc->timer.mictime();
    if (time < (size_t) ARG_(0))
    {
        std::this_thread::sleep_for(std::chrono::microseconds((size_t) ARG_(0) - time));
    }
    proc->timer.start();
}
// END: MISC

#undef ARG_
#undef ARGN_
#undef CMD_
#undef POP_
#undef PUSH_

/*=================================================================================================================*/
