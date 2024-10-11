#include <stdio.h>
#include <cassert>
#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <string.h>
#include "queue.h"

static const size_t CAP_MULTIPLICATOR = 2;  // >= 2 !!!
static const size_t EXPONENTIAL_LIMIT = 1e6;
static const size_t MAX_CAP = 1e8;
static const size_t DUMP_MAX_DATA = 100;

struct queue{
    size_t size;
    size_t start;
    size_t base_capacity;
    size_t capacity;
    size_t elm_width;
    void* data;
};

typedef enum {
    EXPAND = 0,
    SHRINK = 1
} Cap_modification;

static void queue_ctor (queue_t* que, size_t elm_width, size_t base_capacity = QUEUE_DEFAULT_CAP);
static void queue_dtor (queue_t* que);

static void queue_push_resize (queue_t* que, size_t n_of_elms);
static void queue_pop_resize (queue_t* que);

static void queue_resize(queue_t* que, size_t new_cap);
static void queue_cut_excess(queue_t* que);

queue_t* queue_new (size_t elm_width, size_t base_capacity)
{
    assert("queue_new:" && elm_width != 0);
    queue_t* que = nullptr;
    que = (queue_t*)calloc(1, sizeof(queue_t));
    if (!que)
    {
        fprintf(stderr, "Error: allocating memory for queue_t failed.\n");
        return nullptr;
    }
    queue_ctor(que, elm_width, base_capacity);
    return que;
}
static void queue_ctor (queue_t* que, size_t elm_width, size_t base_capacity)
{
    assert(que && "queue_ctor(nullptr, ...)");

    queue_dtor(que);

    base_capacity = (base_capacity)? base_capacity : QUEUE_DEFAULT_CAP;
    que->base_capacity = base_capacity;
    que->capacity      = base_capacity;
    que->elm_width     = elm_width;
    que->size          = 0;
    que->start         = 0;



    que->data = calloc(base_capacity*elm_width, sizeof(char));
}

void queue_delete (queue_t* que)
{
    queue_dtor(que);
    free(que);
}
static void queue_dtor (queue_t* que)
{
    assert(que && "queue_dtor(nullptr)");
    que->size          = 0;
    que->base_capacity = 0;
    que->capacity      = 0;

    if (!que->data)
        return;

    free((char*)que->data);
    que->data = nullptr;
}

static void queue_push_resize (queue_t* que, size_t n_of_elms)
{
    if (que->size + n_of_elms > que->capacity)
    {
        bool do_exp = que->capacity < EXPONENTIAL_LIMIT;
        size_t new_cap = ((do_exp)? que->capacity * CAP_MULTIPLICATOR : que->capacity + EXPONENTIAL_LIMIT);
        if (new_cap < que->size + n_of_elms)
        {
            new_cap += n_of_elms;
        }

        if (new_cap > MAX_CAP)
        {
            if (new_cap - que->start > MAX_CAP)
            {
                assert(0 && "DON'T PUSH ME THAT HARD");
            }
            else
            {
                queue_cut_excess(que);
            }
        }

        que->capacity = new_cap;

        queue_resize(que, new_cap);
    }
}

static void queue_pop_resize (queue_t* que)
{
    size_t size_target = 1;
    size_t new_cap = 1;

    if (que->capacity < CAP_MULTIPLICATOR*EXPONENTIAL_LIMIT)
    {
        size_target = que->capacity / (CAP_MULTIPLICATOR * CAP_MULTIPLICATOR);
        new_cap = que->capacity / CAP_MULTIPLICATOR;
    }
    else
    {
        size_target = que->capacity - 2*EXPONENTIAL_LIMIT;
        new_cap = que->capacity - EXPONENTIAL_LIMIT;
    }

    if (que->size <= size_target)
    {
        queue_resize(que, new_cap);
    }
}

static void queue_resize(queue_t* que, size_t new_cap)
{
    new_cap = new_cap? new_cap : QUEUE_DEFAULT_CAP;
    que->capacity = new_cap;

    void* new_ptr = realloc((char*)que->data, new_cap * que->elm_width);

    if (!new_ptr)
    {
        assert(0 && "REALLOC_NULL");
    }
    que->data = new_ptr;
}

static void queue_cut_excess(queue_t* que)
{
    if (que->start == 0)
        return;

    memcpy(que->data, (char*)que->data + que->start, que->size - que->start);

    que->size -= que->start;
    queue_resize(que, que->size);
}

void queue_push (queue_t* que, const void* value, size_t n_of_elms)
{
    queue_push_resize (que, n_of_elms);

    memcpy((char*)que->data + que->size*que->elm_width, value, que->elm_width*n_of_elms);
    que->size += n_of_elms;
}

void queue_pop (queue_t* que, void* dst, size_t n_of_elms)
{
    assert(dst && "queue_pop(): dst = nullptr!!!!!\n");
    if (que->size < que->start + n_of_elms)
    {
        assert(0 && "QUEUE_UNDERFLOW");
    }

    memcpy(dst, (char*)que->data + que->start*que->elm_width, que->elm_width*n_of_elms);
    que->start += n_of_elms;

    queue_pop_resize (que);
}

size_t queue_curr_size (queue_t* que)
{
    return (que->size - que->start);
}

void queue_dump (queue_t* que, code_position_t pos)
{
    fprintf(stderr, "QUEUE_DUMP (called from %s:%d in function %s):\n", pos.file, pos.line, pos.func);

    if (!que)
    {
        fprintf(stderr, "que = nullptr!!!\n\n");
        return;
    }

    fprintf(stderr, "queue_t <que>[%p]:\n", que);

    fprintf(stderr, "elements width = %ld\n", que->elm_width);
    fprintf(stderr, "base capacity = %ld\n", que->base_capacity);
    fprintf(stderr, "capacity = %ld\n", que->capacity);
    fprintf(stderr, "size = %ld\n", que->size);
    fprintf(stderr, "start = %ld\n", que->start);

    if (!que->data)
    {
        fprintf(stderr, "que->data = nullptr!!!\n\n");
        return;
    }

    fprintf(stderr, "data[%p]: \n{\n", que->data);

    if (que->elm_width && que->size)
    {
        unsigned char byte = 0;
        size_t b = 0;
        const size_t max_b = std::min(DUMP_MAX_DATA, que->elm_width*std::min(que->size, que->capacity));
        const int max_n_width = (int)ceil(log10((std::min(DUMP_MAX_DATA / que->elm_width, std::min(que->size, que->capacity)))));

        while (b < max_b)
        {
            fprintf(stderr, "\t[%.*ld] = | ", max_n_width, b / que->elm_width);
            for (size_t i = 0; i < que->elm_width; i++, b++)
            {
                byte = ((unsigned char*)que->data)[b];
                fprintf(stderr, "%.1X%.1X ", byte & 0x0f, (byte & 0xf0) >> 4);
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
