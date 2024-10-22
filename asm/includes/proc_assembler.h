#ifndef PROC_ASSEMBLER_H
#define PROC_ASSEMBLER_H

#include "spu_header.h"
#include "dynarr.h"
#include "stack.h"

#define MAXWRDLEN  15
#define CASE_JUMP(...) case CMD_jmp: case CMD_ja: case CMD_jae: case CMD_jb: case CMD_jbe: case CMD_je: case CMD_jne: {__VA_ARGS__}

typedef struct {
    char name[MAXWRDLEN];
    size_t ip;
} mark_t;

typedef struct {
    size_t mark_ind;
    size_t ip;
} fixup_t;

typedef struct {
    dynarr_t* code;
    dynarr_t* marks;
    stack_t* fixups;
    size_t ip;
} asmblr_state_t;

typedef struct {
    elm_t val;
    size_t ind;
} args_t;

static asmblr_state_t* asmblr_state_new();
static void asmblr_state_ctor(asmblr_state_t* asmblr);
static void asmblr_state_delete(asmblr_state_t* asmblr);
static void asmblr_state_dtor(asmblr_state_t* asmblr);

static void translate (const char* dst_filename, const char* src_filename);
static cmd_code_t get_cmd_code(const char* cmd_word);
static bool add_mark(asmblr_state_t* asmblr, char* name, size_t ip);
static size_t find_mark(asmblr_state_t* asmblr, const char* name);
static bool parse_arg_push_pop(cmd_code_t* cmd_code, args_t* args, char* argword);

static bool execute_fixup(asmblr_state_t* asmblr);
static void write_code(const char* dst_filename, dynarr_t* code);

#endif // PROC_ASSEMBLER_H
