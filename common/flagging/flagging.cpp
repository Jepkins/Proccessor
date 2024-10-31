#include <assert.h>
#include <stdio.h>
#include <string.h>
#include "flagging.h"

GetoptResult getopt_custom(int argc, char ** const argv, const char *optstring, getopt_out* opt_out)
{
    if (opt_out->optind > argc - 1)
        return ARGV_END;

    char* word = argv[opt_out->optind++];
    opt_out->opt = word;

    size_t word_length = strlen(word);

    if (word_length < 2 || word[0] != '-' || (word_length == 2 && word[1] == '-') || (word_length > 2 && word[1] != '-'))
        return NOT_AN_OPT;

    const char* opt_pointer = strstr(optstring, word);

    if (opt_pointer == NULL)
        return OPT_UNKNOWN;

    if (opt_pointer == optstring || opt_pointer[word_length] == '0')
        return INVALID_OPTSTR;

    if (opt_pointer[-1] != ' ' || (opt_pointer[word_length] != ':' && opt_pointer[word_length] != ' '))
        return OPT_UNKNOWN;

    if (opt_pointer[word_length] == ':')
        return getoptarg(argc, argv, opt_out);

    return OPT_FOUND;
}

GetoptResult getoptarg(int argc, char ** const argv, getopt_out* opt_out)
{
    if (opt_out->optind > argc - 1)
        return EXPECTED_ARG;

    if (*(argv + opt_out->optind)[0] == '-')
        return EXPECTED_ARG;

    opt_out->optarg = argv[opt_out->optind++];

    return OPT_FOUND;
}

bool check_opt_flag(GetoptResult opt_flag, getopt_out opt_out)
{
    switch (opt_flag)
    {
        case OPT_FOUND:
        {
            return 1;
        }

        case NOT_AN_OPT:
        {
            printf(
                "Expected option: %s  \n"
                "                 ^   \n"
                "                 |   \n"
                "                 here\n",
                opt_out.opt
            );
            return 0;
        }

        case OPT_UNKNOWN:
        {
            printf("Unknown option: %s\n", opt_out.opt);
            return 0;
        }

        case EXPECTED_ARG:
        {
            printf("Expected argument: %s |<- here\n", opt_out.opt);
            return 0;
        }

        case INVALID_OPTSTR:
        {
            printf("Error, invalid optstr value!\n");
            return 0;
        }

        case ARGV_END:
        {
            return 0;
        }

        default:
        {
            assert(0 && "incorrect opt_flag");
        }

    }
}
