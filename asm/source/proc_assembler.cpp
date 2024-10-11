#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "asm_flagging.h"
#include "proc_assembler.h"

static const size_t MAXCMDLEN = 10;

int main(int argc, char** argv)
{
    StartConfig run_conds;
    asm_setup(argc, argv, &run_conds);
    FILE* istream = fopen(run_conds.input_file, "r");
    FILE* ostream = fopen(run_conds.output_file, "w");

    translate(ostream, istream);

    fclose(istream);
    fclose(ostream);
    return 0;
}

void translate (FILE* dst, FILE* src)
{
    while (!feof(src))
    {
        char cmd_word[MAXCMDLEN] = {0};
        fscanf(src, "%s", cmd_word);

        commands_t cmd = see_cmd(cmd_word);

        if (cmd == UNKNOWN)
        {
            printf("Syntax error (unknown command): %s\n", cmd_word);
            return;
        }

        fprintf(dst, "%d ", cmd);
        switch (cmd)
        {
        case CMD_PUSH:
        {
            elm_t arg = 0;
            fscanf(src, "%lg", &arg);
            fprintf(dst, "%lg\n", arg);
            break;
        }
        default:
        {
            fprintf(dst, "\n");
            break;
        }
        }
        if (cmd == CMD_HLT)
            break;
    }
}

commands_t see_cmd(const char* cmd_word)
{
    if (strcmp(cmd_word, "push") == 0)
        return CMD_PUSH;
    if (strcmp(cmd_word, "inv") == 0)
        return CMD_INV;
    if (strcmp(cmd_word, "dub") == 0)
        return CMD_DUB;
    if (strcmp(cmd_word, "add") == 0)
        return CMD_ADD;
    if (strcmp(cmd_word, "sub") == 0)
        return CMD_SUB;
    if (strcmp(cmd_word, "mul") == 0)
        return CMD_MUL;
    if (strcmp(cmd_word, "div") == 0)
        return CMD_DIV;
    if (strcmp(cmd_word, "sqrt") == 0)
        return CMD_SQRT;
    if (strcmp(cmd_word, "sin") == 0)
        return CMD_SIN;
    if (strcmp(cmd_word, "cos") == 0)
        return CMD_COS;
    if (strcmp(cmd_word, "in") == 0)
        return CMD_IN;
    if (strcmp(cmd_word, "out") == 0)
        return CMD_OUT;
    if (strcmp(cmd_word, "dump") == 0)
        return CMD_DUMP;
    if (strcmp(cmd_word, "hlt") == 0)
        return CMD_HLT;

    return UNKNOWN;
}
