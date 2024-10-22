#ifndef DYNARR_H
#define DYNARR_H

#include <cstddef>

typedef struct dynarr dynarr_t;

#ifndef CODE_POSITION
#define CODE_POSITION
typedef struct {
    const char* file;
    int line;
    const char* func;
} code_position_t;
#define _POS_ {__FILE__, __LINE__, __func__}
#endif // CODE_POSITION

static const size_t DYNARR_DEFAULT_CAP = 16;

dynarr_t* dynarr_new (size_t elm_width, size_t base_capacity = DYNARR_DEFAULT_CAP);
void dynarr_delete (dynarr_t* arr);

void dynarr_push (dynarr_t* arr, const void* value, size_t n_of_elms = 1);

void dynarr_getdata (dynarr_t* arr, size_t index, void* dst, size_t n_of_elms = 1);
void* dynarr_see (dynarr_t* arr, size_t index);
size_t dynarr_curr_size (dynarr_t* arr);

void dynarr_dump (dynarr_t* arr, code_position_t pos);

#endif // DYNARR_H
