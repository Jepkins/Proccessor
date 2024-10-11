#include <stdio.h>
#include "proc_flagging.h"
#include "proccessor.h"
#include "stack.h"

int main(int argc, char** argv)
{
    StartConfig run_conds;
    proc_setup(argc, argv, &run_conds);

    proc_run(run_conds.input_file);
    return 0;
}
