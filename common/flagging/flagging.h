#ifndef FLAGGING_H
#define FLAGGING_H

typedef enum {
    ARGV_END =        -10,
    NOT_AN_OPT =       -1,
    OPT_UNKNOWN =      -2,
    EXPECTED_ARG =     -3,
    OPT_FOUND =         1,
    INVALID_OPTSTR = -100
} GetoptResult;

typedef struct {
    char* opt;
    char* optarg;
    int optind;
} getopt_out;

GetoptResult getopt_custom(int argc, char ** const argv, const char* optstring, getopt_out* opt_out);
GetoptResult getoptarg(int argc, char ** const argv, getopt_out* opt_out);

bool check_opt_flag(GetoptResult opt_flag, getopt_out opt_out);

#endif // FLAGGING_H
