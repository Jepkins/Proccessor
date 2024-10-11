#ifndef PROC_ASSEMBLER_H
#define PROC_ASSEMBLER_H

#include "proccessor.h"

void translate (FILE* dst, FILE* src);
commands_t see_cmd(const char* cmd_word);

#endif // PROC_ASSEMBLER_H
