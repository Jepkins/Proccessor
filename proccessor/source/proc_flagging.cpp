#include "proc_flagging.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

bool proc_setup(int argc, char** argv, StartConfig* run_conds)
{
    const char* optstring = " -i: --input_f: --video ";
    // !!! always starts and ends with ' ', short options must be before long ones, do not use ':' in option names

    getopt_out opt_out = {.optind = 1};

    GetoptResult opt_flag = ARGV_END;

    while ((opt_flag = getopt_custom(argc, argv, optstring, &opt_out)) != ARGV_END)
    {
        if (!check_opt_flag(opt_flag, opt_out))
            run_conds->flagging_error = 1;
        else
            proc_opt_proccessor(opt_out, run_conds);
    }

    if (run_conds->flagging_error)
        return 0;

    return 1;
}

bool proc_opt_proccessor (getopt_out opt_out, StartConfig *run_conds)
{
    if (!strcmp(opt_out.opt, "-i") || !strcmp(opt_out.opt, "--input_f"))
    {
        if (run_conds->is_input_file_selected)
        {
            printf("Can not select multiple files for input! (Do not use multiple -i, --input_f)\n");
            return 0;
        }

        if (strpbrk (opt_out.optarg, ":*?\"<>|") )
        {
            printf("\"%s\" is invalid file name\n", opt_out.optarg);
            return 0;
        }

        run_conds->is_input_file_selected = true;
        run_conds->input_file = opt_out.optarg;

        return 1;
    }

    if (!strcmp(opt_out.opt, "--video"))
    {
        run_conds->do_video = true;

        return 1;
    }

    printf("Option %s found in optstring but not in opt_proccessor()", opt_out.opt);

    return 0;
}
