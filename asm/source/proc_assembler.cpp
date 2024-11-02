#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "asm_flagging.h"
#include "proc_assembler.h"
#include "spu_header.h"
#include "cpp_preprocessor_logic.h"
#include "mytimer.h"

StartConfig run_conds;

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
    timer_cl t;
    t.start();

    asmblr_setup(argc, argv, &run_conds);
    translate(run_conds.output_file, run_conds.input_file);

    printf("TIME = %lu mks\n", t.mictime());
    return 0;
}

#define ERRMSG(msg, ... /* param */) fprintf(stderr,  "[%lu]" msg "\n", get_curr_line(src), __VA_ARGS__)

static int translate (const char* dst_filename, const char* src_filename)
{
    asmblr_state_t* asmblr = asmblr_state_new();

    bool error_met = false;

    FILE* src = fopen(src_filename, "r");
    if (!src)
    {
        fprintf(stderr,  "Could not open file: %s\n", src_filename);
        return -1;
    }

    while (!feof(src))
    {
        char cmd_word[MAXWRDLEN] = {};
        if(!fscanf(src, "%" QUOTE(MAXWRDLEN) "s",  cmd_word))
            continue;

        if (cmd_word[0] == '\0')
            continue;
        if (cmd_word[0] == ';')
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
        args_t args[MAXARGN] = {};
        size_t argn = (size_t)-1;
        if ((argn = (size_t)parse_args(asmblr, cmd_code, args, src)) == (size_t)-1)
        {
            error_met = true;
            continue;
        }
        cmd_code |= (unsigned char)argn << 8;
        dynarr_push(asmblr->code, &cmd_code, sizeof(cmd_code));
        asmblr->ip += sizeof(cmd_code);
        for (int i = 0; argn > 0; i++, argn--)
        {
            dynarr_push(asmblr->code, &args[i].type, sizeof(args[i].type));
            asmblr->ip += sizeof(args[i].type);
            if (args[i].type & MARK_MASK)
            {
                dynarr_push(asmblr->code, &args[i].ind, sizeof(args[i].ind));
                asmblr->ip += sizeof(args[i].ind);
                continue;
            }
            if (args[i].type & IMMEDIATE_MASK)
            {
                dynarr_push(asmblr->code, &args[i].val, sizeof(args[i].val));
                asmblr->ip += sizeof(args[i].val);
            }
            if (args[i].type & REGISTER_MASK)
            {
                dynarr_push(asmblr->code, &args[i].ind, sizeof(args[i].ind));
                asmblr->ip += sizeof(args[i].ind);
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
        fprintf(stderr,  "Error, break\n");
        asmblr_state_delete(asmblr);
        return 1;
    }
    if (dynarr_curr_size(asmblr->code) == 0)
    {
        fprintf(stderr,  "Empty code, break\n");
        return 1;
    }
    if (write_code(dst_filename, asmblr->code) != 0)
        error_met = true;
    asmblr_state_delete(asmblr);

    if (error_met)
        return 1;
    else
        return 0;
}
static void skip_line(FILE* src)
{
    int c;
    do {
        c = fgetc(src);
    } while(c != '\n' && c != EOF);
}

#define DEFCMD_(code, name, seqn, args) if(strcmp(cmd_word, QUOTE(name)) == 0) {return (CAT(CMD_, name) & LAST_BYTE_MASK);}
static cmd_code_t get_cmd_code(const char* cmd_word)
{
    // Expands to   if(strcmp(cmd_word, "unknown") == 0) return (CMD_unknown & LAST_BYTE_MASK);
    //              if(strcmp(cmd_word, "hlt") == 0) return (CMD_hlt & LAST_BYTE_MASK);
    //                                    ...
    #include "defcmd.h"
    return (CMD_unknown & LAST_BYTE_MASK);
}
#undef DEFCMD_


#define DEFREG_(index, name, ...)                                               \
    if(strcmp(reg, QUOTE(name)) == 0)                                           \
    {                                                                           \
        if (reg_met)                                                            \
        {                                                                       \
            ERRMSG("Multiple registers: %s", argword);                          \
            error_met = true;                                                   \
            break;                                                              \
        }                                                                       \
        reg_met = true;                                                         \
        plus = false;                                                           \
        i += 2;                                                                 \
        args[argnum].ind = index;                                               \
        continue;                                                               \
    }

static int parse_args(asmblr_state_t* asmblr, cmd_code_t cmd_code, args_t* args, FILE* src)
{
    unsigned int poss_args = get_possible_args(cmd_code);
    unsigned int max_seqn = get_max_seqn(cmd_code);
    if (max_seqn == 0 || poss_args == 0)
        return 0;

    if (poss_args == 0x01)
    {
        return parse_markarg(asmblr, args, src);
    }

    unsigned int argn_in_seq = 0;
    for (unsigned int poss_args_temp = poss_args; poss_args_temp != 0; poss_args_temp >>= 8)
        argn_in_seq++;

    bool error_met = false;
    bool break_flag = false;
    unsigned int seqnum = 0;
    int argnum = 0;

    for (; !break_flag ; seqnum++)
    {
        for (unsigned int poss_args_temp = poss_args, argnum_in_seq = 0; poss_args_temp != 0; poss_args_temp >>= 8, argnum_in_seq++)
        {
            char argword[MAXWRDLEN] = {};
            fscanf(src, "%" QUOTE(MAXWRDLEN) "s", argword);
            size_t len = strlen(argword);
            assert(!(poss_args_temp & 0x01) && "Invalid cmd list");
            bool do_ram  = false;
            bool imd_met = false;
            bool reg_met = false;
            bool plus = true;

            if (argword[0] == '[')
            {
                if (len < 2 || argword[len-1] != ']')
                {
                    ERRMSG("Expected ']': %s <- here", argword);
                    error_met = true;
                    continue;
                }
                do_ram = true;
                len--;
            }
            if (len == 0)
                break_flag = true;
            for(size_t i = do_ram; i < len;)
            {
                if (argword[i] == '+')
                {
                    if (plus)
                    {
                        ERRMSG("Unexpected '+': %s", argword);
                        error_met = true;
                        break;
                    }
                    plus = true;
                    i++;
                    continue;
                }
                if (!plus)
                {
                    ERRMSG("Unexpected token: %s", argword);
                    error_met = true;
                    break;
                }

                int scan_n = 0;
                sscanf(argword + i, ELM_T_FORMAT "%n", &args[argnum].val, &scan_n);
                if(scan_n)
                {
                    if (imd_met)
                    {
                        ERRMSG("Multiple immediate arguments: %s", argword);
                        error_met = true;
                        break;
                    }
                    if (!plus)
                    {
                        ERRMSG("Use '+': %s", argword);
                        error_met = true;
                        break;
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
                if (len - i >= 2)
                    sscanf(argword + i, "%2s", reg);
                else
                    goto notanarg;

                // Expands to   if(strcmp(reg, "AX") == 0) { ... continue;}
                //              if(strcmp(reg, "BX") == 0) { ... continue;}
                //                                      ...
                #include "defreg.h"

                // Not an argument
                notanarg:
                if (i != do_ram)
                {
                    error_met = true;
                    ERRMSG("Invalid argument: %s", argword);
                }
                if (argnum_in_seq != 0)
                {
                    error_met = true;
                    ERRMSG("Unfinished arguments sequence, last: %s", argword);
                }
                fseek(src, -(int)len, SEEK_CUR);
                break_flag = true;
                break;
            }
            if (break_flag)
                break;

            unsigned char read_arg = (unsigned char) (1 << (imd_met * 1) << (reg_met * 2) << (do_ram * 4));
            if(!(read_arg & poss_args_temp))
            {
                if (read_arg == 0x01)
                    ERRMSG("Expected argument before %s", argword);
                else
                    ERRMSG("Invalid argument: %s", argword);
                continue;
            }

            if (plus)
            {
                ERRMSG("Expected argument after '+': %s", argword);
                error_met = true;
                continue;
            }

            if (imd_met) args[argnum].type |= IMMEDIATE_MASK;
            if (reg_met) args[argnum].type |= REGISTER_MASK;
            if (do_ram)  args[argnum].type |= RAM_MASK;
            if (argnum < MAXARGN - 1) argnum++;
        }
    }
    seqnum--;
    if (seqnum > max_seqn)
    {
        ERRMSG("Too many argument sequences for (code)" CMD_CODE_FORMAT, cmd_code & LAST_BYTE_MASK);
        error_met = true;
    }
    if (seqnum == 0 && max_seqn != 0)
    {
        ERRMSG("Expected arguments for (code)" CMD_CODE_FORMAT, cmd_code & LAST_BYTE_MASK);
        error_met = true;
    }
    if (argnum == MAXARGN - 1)
    {
        ERRMSG("Exceeded MAXARGN for (code)" CMD_CODE_FORMAT, cmd_code & LAST_BYTE_MASK);
        error_met = true;
    }
    if (error_met)
        return -1;
    return argnum;
}
#undef DEFREG_

static int parse_markarg(asmblr_state_t* asmblr, args_t* args, FILE* src)
{
    char jump_mark[MAXWRDLEN] = {};
    if (!fscanf(src, "%" QUOTE(MAXWRDLEN) "s", jump_mark) || jump_mark[0] != ':')
    {
        fseek(src, -(int)strlen(jump_mark), SEEK_CUR);
        ERRMSG("Expected mark, got: %s", jump_mark);
        return -1;
    }
    size_t mark_ind = find_mark(asmblr, jump_mark);
    if (mark_ind == (size_t)-1)
    {
        if (!add_mark(asmblr, jump_mark, (size_t)-1))
        {
            return -1;
        }
        mark_ind = dynarr_curr_size(asmblr->marks) - 1;
    }
    size_t mark_ip = ((mark_t*)dynarr_see(asmblr->marks, mark_ind))->ip;


    if (mark_ip == (size_t)-1)
    {
        fixup_t new_fixup = {mark_ind, asmblr->ip + sizeof(cmd_code_t) + sizeof(argtype_t)};
        stack_push(asmblr->fixups, &new_fixup);
    }
    args[0].type |= MARK_MASK;
    args[0].ind   = mark_ip;
    return 1;
}

static bool add_mark(asmblr_state_t* asmblr, char* name, size_t ip)
{
    size_t i = find_mark(asmblr, name);

    if (i == (size_t)-1)
    {
        mark_t new_mark = {{}, ip};
        strncpy(new_mark.name, name, MAXWRDLEN);
        dynarr_push(asmblr->marks, &new_mark);
    }
    else
    {
        if (((mark_t*)dynarr_see(asmblr->marks, i))->ip != (size_t)-1)
        {
            fprintf(stderr,  "Multiple mark definition: %s\n", ((mark_t*)dynarr_see(asmblr->marks, i))->name);
            return 0;
        }
        else
        {
            ((mark_t*)dynarr_see(asmblr->marks, i))->ip = ip;
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

    if(i == marks_num)
        return (size_t)-1;

    return i;
}

#define DEFCMD_(code, name, argn, args) case code: {return args;}
static unsigned int get_possible_args(cmd_code_t code)
{
    code = code & LAST_BYTE_MASK;
    switch (code)
    {
        // Expands to  case 0xff: {return 0x00;}
        //             case 0x00: {return 0x01;}
        //                        ...
        #include "defcmd.h"
        default: return 0;
    }
}
#undef DEFCMD_

#define DEFCMD_(code, name, seqn, args) case code: {return seqn;}
static unsigned int get_max_seqn(cmd_code_t code)
{
    code = code & LAST_BYTE_MASK;
    switch (code)
    {
        // Expands to  case 0xff: {return  0;}
        //             case 0x01: {return 10;}
        //                        ...
        #include "defcmd.h"
        default: return 0;
    }
}
#undef DEFCMD_

static bool execute_fixup(asmblr_state_t* asmblr)
{
    if (dynarr_curr_size(asmblr->code) == 0)
        return 1;
    size_t n_of_fixups = stack_curr_size(asmblr->fixups);
    for (size_t i = 0; i < n_of_fixups; i++)
    {
        fixup_t fixup = {};
        stack_pop(asmblr->fixups, &fixup);
        mark_t* mark = (mark_t*)dynarr_see(asmblr->marks, fixup.mark_ind);
        size_t fixed_ip = mark->ip;

        if (fixed_ip == (size_t)-1)
        {
            fprintf(stderr,  "Mark used but not defined: %s\n", mark->name);
            return 0;
        }
        *(size_t*)dynarr_see(asmblr->code, fixup.ip) = fixed_ip;
    }
    return 1;
}

static int write_code(const char* dst_filename, dynarr_t* code)
{
    FILE* dst = fopen(dst_filename, "wb");
    if (!dst)
    {
        fprintf(stderr,  "Could not open file: %s\n", dst_filename);
        return -1;
    }

    size_t code_size = dynarr_curr_size(code);
    spu_header_t head;
    head.code_size = code_size;
    fwrite(&head, sizeof(head), 1, dst);
    fwrite(dynarr_see(code, 0), sizeof(char) , code_size, dst);
    if (ferror(dst))
    {
        fprintf(stderr, "Failed to write code, ferror code = %d", ferror(dst));
        fclose(dst);
        return 1;
    }
    fclose(dst);
    return 0;
}

static size_t get_curr_line(FILE* fstream)
{
    long int curr_shift = ftell(fstream);
    fseek(fstream, 0, SEEK_SET);
    size_t line = 1;
    for (int i = 0; i < curr_shift; i++)
    {
        if (fgetc(fstream) == '\n')
            line++;
    }
    fseek(fstream, curr_shift, SEEK_SET);
    return line;
}

#undef ERRMSG
