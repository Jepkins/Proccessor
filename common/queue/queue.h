#ifndef QUEUE_H
#define QUEUE_H

#include <cstddef>

typedef struct queue queue_t;

#ifndef CODE_POSITION
#define CODE_POSITION
typedef struct {
    const char* file;
    int line;
    const char* func;
} code_position_t;
#define _POS_ {__FILE__, __LINE__, __func__}
#endif // CODE_POSITION

static const size_t QUEUE_DEFAULT_CAP = 16;

queue_t* queue_new (size_t elm_width, size_t base_capacity = QUEUE_DEFAULT_CAP);
void queue_delete (queue_t* que);

void queue_push (queue_t* que, const void* value, size_t n_of_elms = 1);
void queue_pop (queue_t* que, void* dst, size_t n_of_elms = 1);

size_t queue_curr_size (queue_t* que);

void queue_dump (queue_t* que, code_position_t pos);

#endif // QUEUE_H
