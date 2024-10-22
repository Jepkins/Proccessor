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

printf("caught %s\n", cmd_word);
        if (strcmp(cmd_word, "") == 0)
            break;
        if (cmd_word[0] == ':')
        {
            if(!add_mark(asmblr, cmd_word, asmblr->ip))
                error_met = true;

            continue;
        }

        cmd_code_t cmd_code = get_cmd_code(cmd_word);
        if ((cmd_code & LAST_BYTE_MASK) == CMD_unknown)
        {
            printf("Syntax error (unknown command): %s\n", cmd_word);
            error_met = true;
            continue;
        }

        args_t args = {};

        switch (cmd_code & LAST_BYTE_MASK)
        {
        case CMD_push:
        case CMD_pop:
        {
            char argword[MAXWRDLEN] = {};
            fscanf(src, "%s", argword);
            if (!parse_arg_push_pop(&cmd_code, &args, argword))
            {
                error_met = true;
                continue;
            }
            break;
        }
        CASE_JUMP
        (
            cmd_code = cmd_code ^ IMMEDIATE_MASK;
            char jump_mark[MAXWRDLEN] = {};
            if (!fscanf(src, "%" QUOTE(MAXWRDLEN) "s", jump_mark) || jump_mark[0] != ':')
            {
                printf("Expected mark after %s\n", cmd_word);
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
                args.val = (elm_t)-1;
            }
            else
            {
                args.val = (elm_t)mark_ip;
            }
            break;
        )
        default:
        {
            break;
        }
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
    fclose(src);

    if (error_met)
    {
        printf("Error, break\n");
        asmblr_state_delete(asmblr);
        return;
    }
    if (execute_fixup(asmblr))
    {
        write_code(dst_filename, asmblr->code);
    }
    asmblr_state_delete(asmblr);
}

#define ASMBLR_SEE_CMD_IF_PUSHEND(code, name, ...) __VA_ARGS__ if(strcmp(cmd_word, QUOTE(name)) == 0) {return (PRIMITIVE_CAT(CMD_, name) & LAST_BYTE_MASK);}
static cmd_code_t get_cmd_code(const char* cmd_word)
{
    // Expands to   if(strcmp(cmd_word, "unknown") == 0) return (CMD_unknown & LAST_BYTE_MASK);
    //              if(strcmp(cmd_word, "hlt") == 0) return (CMD_hlt & LAST_BYTE_MASK);
    //                                    ...
    EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, ASMBLR_SEE_CMD_IF_PUSHEND, PROC_CMD_LIST, )))
    return (CMD_unknown & LAST_BYTE_MASK);
}

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
            printf("Multiple mark definition: %s\n", ((mark_t*)dynarr_see(asmblr->marks, i))->name);
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

#define PARSE_ARG_PUSH_POP_IF_PUSHEND(index, name, ...) __VA_ARGS__ if(strcmp(reg, QUOTE(name)) == 0) {args->ind = index; continue;}

static bool parse_arg_push_pop(cmd_code_t* cmd_code, args_t* args, char* argword)
{
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
            printf("Expected ']': %s <- here\n", argword);
            return 0;
        }
        do_ram = true;
        len -= 2;
        argword++;
    }
    for(size_t i = 0; i < len;)
    {
        // printf("argword = %s\n", argword);
        if (argword[i] == '+')
        {
            if (plus)
            {
                printf("Multiple '+': %s\n", argword);
                return 0;
            }
            plus = true;
            i++;
            continue;
        }

        int scan_n = 0;
        sscanf(argword + i, ELM_T_FORMAT "%n", &args->val, &scan_n);
        if(scan_n)
        {
            if (imd_met)
            {
                printf("Multiple imediate arguments: %s\n", argword);
                return 0;
            }
            if (!plus)
            {
                printf("Use '+': %s\n", argword);
            }
            else
            {
                imd_met = true;
                plus = false;
                i += (size_t)scan_n;
                continue;
            }
        }

        if (reg_met)
        {
            printf("Multiple registers: %s\n", argword);
            return 0;
        }
        if (!plus)
        {
            printf("Use '+': %s\n", argword);
        }
        reg_met = true;
        plus = false;
        char reg[3] = {};
        sscanf(argword + i, "%2s", reg);
        i += 2;

        // Expands to   if(strcmp(reg, "AX") == 0) {args[1] = 0x01; continue;}
        //              if(strcmp(reg, "BX") == 0) {args[1] = 0x02; continue;}
        //                                      ...
        EXPAND(DEFER(DELETE_FIRST)(WHILE(NOT_END, PARSE_ARG_PUSH_POP_IF_PUSHEND, PROC_REGS_LIST, )))

        printf("Invalid argument: %s\n", argword);
        return 0;
    }
    if (imd_met) *cmd_code = *cmd_code ^ IMMEDIATE_MASK;
    if (reg_met) *cmd_code = *cmd_code ^ REGISTER_MASK;
    if (do_ram)  *cmd_code = *cmd_code ^ RAM_MASK;

    if ((*cmd_code & LAST_BYTE_MASK) == CMD_pop &&
        (( reg_met && imd_met && !do_ram) || (!reg_met && imd_met && !do_ram)))
    {
        printf("Invalid pop argument: %s\n", argword);
        return 0;
    }

    return 1;
}

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
            printf("Mark used but not defined: %s\n", mark->name);
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
