#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "asm_flagging.h"
#include "proc_assembler.h"
#include "spu_header.h"
#include "cpp_preprocessor_logic.h"

static asmblr_state_t* asmblr_state_new()
{
    asmblr_state_t* asmblr = (asmblr_state_t*)calloc(1, sizeof(*asmblr));
    asmblr_state_ctor(asmblr);
    return asmblr;
}
static void asmblr_state_ctor(asmblr_state_t* asmblr)
{
    asmblr_state_dtor(asmblr);
    asmblr->marks  = dynarr_new(sizeof(mark_t));
    asmblr->fixups = stack_new(sizeof(fixup_t));
    asmblr->code   = dynarr_new(sizeof(char));
    asmblr->ip = 0;
}
static void asmblr_state_delete(asmblr_state_t* asmblr)
{
    asmblr_state_dtor(asmblr);
    free(asmblr);
}
static void asmblr_state_dtor(asmblr_state_t* asmblr)
{
    if (asmblr)
    {
        if (asmblr->marks)
        {
            dynarr_delete(asmblr->marks);
            asmblr->marks = nullptr;
        }
        if (asmblr->fixups)
        {
            stack_delete(asmblr->fixups);
            asmblr->fixups = nullptr;
        }
        if (asmblr->code)
        {
            dynarr_delete(asmblr->code);
            asmblr->code = nullptr;
        }
    }
}

int main(int argc, char** argv)
{
    StartConfig run_conds;
    asmblr_setup(argc, argv, &run_conds);

    translate(run_conds.output_file, run_conds.input_file);

    return 0;
}

#define ERRMSG(msg, ... /* param */) printf("[%3lu]" msg "\n", asmblr->ip, __VA_ARGS__)

static void translate (const char* dst_filename, const char* src_filename)
{
    asmblr_state_t* asmblr = asmblr_state_new();

    bool error_met = false;

    FILE* src = fopen(src_filename, "r");
    if (!src)
    {
        printf("Could not open file: %s\n", src_filename);
        return;
    }

    while (!feof(src))
    {
        char cmd_word[MAXWRDLEN] = {0};
        if(!fscanf(src, "%" QUOTE(MAXWRDLEN) "s",  cmd_word))
            break;
        if (strcmp(cmd_word, "") == 0)
            break;

        if (strcmp(cmd_word, ";") == 0)
        {
            skip_line(src);
            continue;
        }
        if (cmd_word[0] == ':')
        {
            if(!add_mark(asmblr, cmd_word, asmblr->ip))
                error_met = true;

            continue;
        }

        cmd_code_t cmd_code = get_cmd_code(cmd_word);
        if ((cmd_code & LAST_BYTE_MASK) == CMD_unknown)
        {
            ERRMSG("Syntax error (unknown command): %s", cmd_word);
            error_met = true;
            continue;
        }

        switch (cmd_code & LAST_BYTE_MASK)
        {
        CASE_JUMP_CALL
        (
            cmd_code = cmd_code ^ INDEX_MASK;
            char jump_mark[MAXWRDLEN] = {};
            if (!fscanf(src, "%" QUOTE(MAXWRDLEN) "s", jump_mark) || jump_mark[0] != ':')
            {
                ERRMSG("Expected mark after %s", cmd_word);
                error_met = true;
                continue;
            }
            size_t mark_ind = find_mark(asmblr, jump_mark);
            if (mark_ind == (size_t)-1)
            {
                add_mark(asmblr, jump_mark, (size_t)-1);
                mark_ind = dynarr_curr_size(asmblr->marks) - 1;
            }
            size_t mark_ip = ((mark_t*)dynarr_see(asmblr->marks, mark_ind))->ip;

            if (mark_ip == (size_t)-1)
            {
                fixup_t new_fixup = {mark_ind, asmblr->ip + sizeof(cmd_code)};
                stack_push(asmblr->fixups, &new_fixup);
            }
            asmblr->ip += sizeof(cmd_code);
            dynarr_push(asmblr->code, &cmd_code, sizeof(cmd_code));
            asmblr->ip += sizeof(mark_ip);
            dynarr_push(asmblr->code, &mark_ip, sizeof(mark_ip));
            break;
        )
        default:
        {
            args_t args = {};
            if (!parse_arg(asmblr, &cmd_code, &args, src))
            {
                error_met = true;
                continue;
            }
            asmblr->ip += sizeof(cmd_code);
            dynarr_push(asmblr->code, &cmd_code, sizeof(cmd_code));
            if (cmd_code & IMMEDIATE_MASK)
            {
                asmblr->ip += sizeof(args.val);
                dynarr_push(asmblr->code, &args.val, sizeof(args.val));
            }
            if (cmd_code &  REGISTER_MASK)
            {
                asmblr->ip += sizeof(args.ind);
                dynarr_push(asmblr->code, &args.ind, sizeof(args.ind));
            }
        }
        }
    }
    fclose(src);

    if (!execute_fixup(asmblr))
    {
        error_met = true;
    }
    if (error_met)
    {
        printf("Error, break\n");
        asmblr_state_delete(asmblr);
        return;
    }
    write_code(dst_filename, asmblr->code);
    asmblr_state_delete(asmblr);
}
static void skip_line(FILE* src)
{
    int c;
    do {
        c = fgetc(src);
    } while(c != '\n' && c != EOF);
}

#define ASMBLR_STRCMP_PUSHEND(code, name, args, ...) __VA_ARGS__ if(strcmp(cmd_word, QUOTE(name)) == 0) {return (CAT(CMD_, name) & LAST_BYTE_MASK);}
static cmd_code_t get_cmd_code(const char* cmd_word)
{
    // Expands to   if(strcmp(cmd_word, "unknown") == 0) return (CMD_unknown & LAST_BYTE_MASK);
    //              if(strcmp(cmd_word, "hlt") == 0) return (CMD_hlt & LAST_BYTE_MASK);
    //                                    ...
    EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, ASMBLR_STRCMP_PUSHEND, PROC_CMD_LIST, )))
    return (CMD_unknown & LAST_BYTE_MASK);
}
#undef ASMBLR_STRCMP_PUSHEND

static bool add_mark(asmblr_state_t* asmblr, char* name, size_t ip)
{
    size_t i = find_mark(asmblr, name);

    if (i == (size_t)-1)
    {
        mark_t new_mark = {{}, ip};
        strcpy(new_mark.name, name);
        dynarr_push(asmblr->marks, &new_mark);
    }
    else
    {
        if (((mark_t*)dynarr_see(asmblr->marks, i))->ip != (size_t)-1)
        {
            ERRMSG("Multiple mark definition: %s", ((mark_t*)dynarr_see(asmblr->marks, i))->name);
            return 0;
        }
        else
        {
            ((mark_t*)dynarr_see(asmblr->marks, i))->ip = ip;
            // printf("add_mark_ip = %lu\n", ip);
        }
    }
    return 1;
}

static size_t find_mark(asmblr_state_t* asmblr, const char* name)
{
    size_t i = 0;
    size_t marks_num = dynarr_curr_size(asmblr->marks);

    while (i < marks_num && strcmp(name, ((mark_t*)dynarr_see(asmblr->marks, i))->name) != 0)
        i++;
// printf("hi, i'm find_mark() i = %lu\n", i);
    if(i == marks_num)
        return (size_t)-1;

    return i;
}

#define PARSE_ARG_PUSH_POP_IF_PUSHEND(index, name, ...) __VA_ARGS__             \
    if(strcmp(reg, QUOTE(name)) == 0)                                           \
    {                                                                           \
        if (reg_met)                                                            \
        {                                                                       \
            ERRMSG("Multiple registers: %s", argword);                          \
            return 0;                                                           \
        }                                                                       \
        reg_met = true;                                                         \
        plus = false;                                                           \
        i += 2;                                                                 \
        args->ind = index;                                                      \
        continue;                                                               \
    }

static bool parse_arg(asmblr_state_t* asmblr, cmd_code_t* cmd_code, args_t* args, FILE* src)
{
    unsigned char poss_args = get_possible_args(*cmd_code);
    assert(poss_args && "invalid cmd_code");
    if (poss_args == 0x01)
        return 1;

    char argword[MAXWRDLEN] = {};
    fscanf(src, "%s", argword);
    size_t len = strlen(argword);
    assert(len != 0 && "invalid argword");
    bool do_ram  = false;
    bool imd_met = false;
    bool reg_met = false;
    bool plus = true;

    if (argword[0] == '[')
    {
        if (len < 2 || argword[len-1] != ']')
        {
            ERRMSG("Expected ']': %s <- here", argword);
            return 0;
        }
        do_ram = true;
        len -= 1;
    }
    for(size_t i = do_ram; i < len;)
    {
        if (argword[i] == '+')
        {
            if (plus)
            {
                ERRMSG("Multiple '+': %s", argword);
                return 0;
            }
            plus = true;
            i++;
            continue;
        }
        if (!plus)
        {
            ERRMSG("Use '+': %s", argword);
            return 0;
        }

        int scan_n = 0;
        sscanf(argword + i, ELM_T_FORMAT "%n", &args->val, &scan_n);
        if(scan_n)
        {
            if (imd_met)
            {
                ERRMSG("Multiple immediate arguments: %s", argword);
                return 0;
            }
            if (!plus)
            {
                ERRMSG("Use '+': %s", argword);
                return 0;
            }
            else
            {
                imd_met = true;
                plus = false;
                i += (size_t)scan_n;
                continue;
            }
        }

        char reg[3] = {};
        sscanf(argword + i, "%2s", reg);

        // Expands to   if(strcmp(reg, "AX") == 0) { ... continue;}
        //              if(strcmp(reg, "BX") == 0) { ... continue;}
        //                                      ...
        EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, PARSE_ARG_PUSH_POP_IF_PUSHEND, PROC_REGS_LIST, )))

        // Not an argument
        break;
    }

    unsigned char read_arg = (unsigned char) (1 << (imd_met * 1) << (reg_met * 2) << (do_ram * 4));
    if(!(read_arg & poss_args))
    {
        if (read_arg == 1)
            ERRMSG("Expected argument before %s", argword);
        else
            ERRMSG("Invalid argument: %s", argword);
    }

    if (plus)
    {
        ERRMSG("Expected argument after '+': %s", argword);
        return 0;
    }
    if (imd_met) *cmd_code = *cmd_code ^ IMMEDIATE_MASK;
    if (reg_met) *cmd_code = *cmd_code ^ REGISTER_MASK;
    if (do_ram)  *cmd_code = *cmd_code ^ RAM_MASK;

    return 1;
}
#undef PARSE_ARG_PUSH_POP_IF_PUSHEND

#define GET_POSS_ARGS_PUSHEND(code, name, args, ...) __VA_ARGS__ case code: {return args;}
static unsigned char get_possible_args(cmd_code_t code)
{
    code = code & LAST_BYTE_MASK;
    switch (code)
    {
        // Expands to  case 0xff: {return 0x00;}
        //             case 0x00: {return 0x01;}
        //                        ...
        EXPAND(DEFER(DELETE_FIRST_1)(WHILE(NOT_END, GET_POSS_ARGS_PUSHEND, PROC_CMD_LIST, )))
        default: return 0;
    }
}
#undef GET_POSS_ARGS_PUSHEND

static bool execute_fixup(asmblr_state_t* asmblr)
{
    size_t n_of_fixups = stack_curr_size(asmblr->fixups);
    for (size_t i = 0; i < n_of_fixups; i++)
    {
        fixup_t fixup = {};
        stack_pop(asmblr->fixups, &fixup);
        mark_t* mark = (mark_t*)dynarr_see(asmblr->marks, fixup.mark_ind);
        // printf("fix: ip = %lu, ind = %lu\n", mark->ip, fixup.mark_ind);
        size_t fixed_ip = mark->ip;

        if (fixed_ip == (size_t)-1)
        {
            ERRMSG("Mark used but not defined: %s", mark->name);
            return 0;
        }
        *(size_t*)dynarr_see(asmblr->code, fixup.ip) = fixed_ip;
    }
    return 1;
}

static void write_code(const char* dst_filename, dynarr_t* code)
{
    FILE* dst = fopen(dst_filename, "wb");
    if (!dst)
    {
        printf("Could not open file: %s\n", dst_filename);
        return;
    }

    size_t code_size = dynarr_curr_size(code);
    spu_header_t head;
    head.code_size = code_size;
    fwrite(&head, sizeof(head), 1, dst);
    fwrite(dynarr_see(code, 0), sizeof(char) , code_size, dst);
}

#undef ERRMSG
