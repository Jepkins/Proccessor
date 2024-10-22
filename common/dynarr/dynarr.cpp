#include <stdio.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string.h>
#include "dynarr.h"

static const size_t CAP_MULTIPLICATOR = 2;  // >= 2 !!!
static const size_t EXPONENTIAL_LIMIT = 1e6;
static const size_t MAX_CAP = 1e8;
static const size_t DUMP_MAX_DATA = 100;

struct dynarr{
    size_t size;
    size_t base_capacity;
    size_t capacity;
    size_t elm_width;
    void* data;
};

typedef enum {
    EXPAND = 0,
    SHRINK = 1
} Cap_modification;

static void dynarr_ctor (dynarr_t* arr, size_t elm_width, size_t base_capacity = DYNARR_DEFAULT_CAP);
static void dynarr_dtor (dynarr_t* arr);

static void dynarr_push_resize (dynarr_t* arr, size_t n_of_elms);

static void dynarr_resize(dynarr_t* arr, size_t new_cap);

dynarr_t* dynarr_new (size_t elm_width, size_t base_capacity)
{
    assert("dynarr_new:" && elm_width != 0);
    dynarr_t* arr = nullptr;
    arr = (dynarr_t*)calloc(1, sizeof(dynarr_t));
    if (!arr)
    {
        fprintf(stderr, "Error: allocating memory for dynarr_t failed.\n");
        return nullptr;
    }
    dynarr_ctor(arr, elm_width, base_capacity);
    return arr;
}
static void dynarr_ctor (dynarr_t* arr, size_t elm_width, size_t base_capacity)
{
    assert(arr && "dynarr_ctor(nullptr, ...)");

    dynarr_dtor(arr);

    base_capacity = (base_capacity)? base_capacity : DYNARR_DEFAULT_CAP;
    arr->base_capacity = base_capacity;
    arr->capacity      = base_capacity;
    arr->elm_width     = elm_width;
    arr->size          = 0;

    arr->data = calloc(base_capacity*elm_width, sizeof(char));
}

void dynarr_delete (dynarr_t* arr)
{
    dynarr_dtor(arr);
    free(arr);
}
static void dynarr_dtor (dynarr_t* arr)
{
    assert(arr && "dynarr_dtor(nullptr)");
    arr->size          = 0;
    arr->base_capacity = 0;
    arr->capacity      = 0;

    if (!arr->data)
        return;

    free((char*)arr->data);
    arr->data = nullptr;
}

static void dynarr_push_resize (dynarr_t* arr, size_t n_of_elms)
{
    if (arr->size + n_of_elms > arr->capacity)
    {
        bool do_exp = arr->capacity < EXPONENTIAL_LIMIT;
        size_t new_cap = ((do_exp)? arr->capacity * CAP_MULTIPLICATOR : arr->capacity + EXPONENTIAL_LIMIT);
        if (new_cap < arr->size + n_of_elms)
        {
            new_cap += n_of_elms;
        }

        if (new_cap > MAX_CAP)
        {
            assert(0 && "DON'T PUSH ME THAT HARD");
        }

        arr->capacity = new_cap;

        dynarr_resize(arr, new_cap);
    }
}

static void dynarr_resize(dynarr_t* arr, size_t new_cap)
{
    new_cap = new_cap? new_cap : DYNARR_DEFAULT_CAP;
    arr->capacity = new_cap;

    void* new_ptr = realloc((char*)arr->data, new_cap * arr->elm_width);

    if (!new_ptr)
    {
        assert(0 && "REALLOC_NULL");
    }
    arr->data = new_ptr;
}

void dynarr_push (dynarr_t* arr, const void* value, size_t n_of_elms)
{
    dynarr_push_resize (arr, n_of_elms);

    memcpy((char*)arr->data + arr->size*arr->elm_width, value, arr->elm_width*n_of_elms);
    arr->size += n_of_elms;
}

void dynarr_getdata (dynarr_t* arr, size_t index, void* dst, size_t n_of_elms)
{
    assert(arr && "dynarr_see(): arr = nullptr!!!!!");
    assert(dst && "dynarr_getdata(): dst = nullptr!!!!!");

    if (index + n_of_elms > arr->size)
    {
        printf("dynarr_getdata(): Out of boundaries: size = %lu, index = %lu, n_of_elms = %lu\n", arr->size, index, n_of_elms);
        return;
    }
    memcpy(dst, (char*)arr->data + index * arr->elm_width, arr->elm_width*n_of_elms);
}
void* dynarr_see (dynarr_t* arr, size_t index)
{
    assert(arr && "dynarr_see(): arr = nullptr!!!!!");

    if (index >= arr->size)
    {
        printf("dynarr_see(): Out of boundaries: size = %lu, index = %lu\n", arr->size, index);
        return nullptr;
    }
    return (char*)arr->data + index * arr->elm_width;
}

size_t dynarr_curr_size (dynarr_t* arr)
{
    return (arr->size);
}

void dynarr_dump (dynarr_t* arr, code_position_t pos)
{
    fprintf(stderr, "DYNARR_DUMP (called from %s:%d in function %s):\n", pos.file, pos.line, pos.func);

    if (!arr)
    {
        fprintf(stderr, "arr = nullptr!!!\n\n");
        return;
    }

    fprintf(stderr, "dynarr_t <arr>[%p]:\n", arr);

    fprintf(stderr, "elements width = %ld\n", arr->elm_width);
    fprintf(stderr, "base capacity = %ld\n", arr->base_capacity);
    fprintf(stderr, "capacity = %ld\n", arr->capacity);
    fprintf(stderr, "size = %ld\n", arr->size);

    if (!arr->data)
    {
        fprintf(stderr, "arr->data = nullptr!!!\n\n");
        return;
    }

    fprintf(stderr, "data[%p]: \n{\n", arr->data);

    if (arr->elm_width && arr->size)
    {
        unsigned char byte = 0;
        size_t b = 0;
        const size_t max_b = std::min(DUMP_MAX_DATA, arr->elm_width*std::min(arr->size, arr->capacity));
        const int max_n_width = (int)ceil(log10((double)(std::min(DUMP_MAX_DATA / arr->elm_width, std::min(arr->size, arr->capacity)))));

        while (b < max_b)
        {
            fprintf(stderr, "\t[%.*ld] = | ", max_n_width, b / arr->elm_width);
            for (size_t i = 0; i < arr->elm_width; i++, b++)
            {
                byte = ((unsigned char*)arr->data)[b];
                fprintf(stderr, "%.2X ", byte);
            }
            fprintf(stderr, "|\n");
        }
        if (b >= DUMP_MAX_DATA)
        {
            fprintf(stderr, "\t... (too many members)\n");
        }
    }
    fprintf(stderr, "}\n\n");
}
