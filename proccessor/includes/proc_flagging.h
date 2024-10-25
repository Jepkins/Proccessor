#ifndef PROC_FLAGGING_H
#define PROC_FLAGGING_H

#include "flagging.h"

static const char DEFAULT_CODE_FILE[20] = "code.txt";

typedef struct {
    bool is_input_file_selected = false;
    const char* input_file = DEFAULT_CODE_FILE;

    bool do_video = false;

    bool flagging_error = false;
} StartConfig;

bool proc_setup(int argc, char** argv, StartConfig* run_conds);
bool proc_opt_proccessor (getopt_out opt_out, StartConfig *run_conds);

#endif // PROC_FLAGGING_H
