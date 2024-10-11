#ifndef ASM_FLAGGING_H
#define ASM_FLAGGING_H

#include "flagging.h"

static const char DEFAULT_IFILE[20] = "source_code.txt";
static const char DEFAULT_OFILE[20] = "code.txt";

typedef struct {
    bool is_input_file_selected = false;
    const char* input_file = DEFAULT_IFILE;

    bool is_output_file_selected = false;
    const char* output_file = DEFAULT_OFILE;

    bool flagging_error = false;
} StartConfig;

bool asm_setup(int argc, char** argv, StartConfig* run_conds);
bool asm_opt_proccessor (getopt_out opt_out, StartConfig *run_conds);

#endif // ASM_FLAGGING_H
