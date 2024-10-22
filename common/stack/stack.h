#ifndef STACK_H
#define STACK_H

#include <cstddef>

typedef struct stack stack_t;

#ifndef CODE_POSITION
#define CODE_POSITION
typedef struct {
    const char* file;
    int line;
    const char* func;
} code_position_t;
#define _POS_ {__FILE__, __LINE__, __func__}
#endif // CODE_POSITION

// #define NO_DATA_HASHING
#ifdef DEBUG
#define DO_HASH
#define DO_CANARY
#define _DBG(...) __VA_ARGS__
#define IF_DO_CANARY(...) __VA_ARGS__
#define IF_DO_HASH(...) __VA_ARGS__
#define _STACK_ASSERT_(stk) {stack_assert(stk, _POS_);}
#else
#define _DBG(...)
#define IF_DO_CANARY(...)
#define IF_DO_HASH(...)
#define _STACK_ASSERT_(stk) if (stack_err(stk)) assert(0 && "stack_error");
#endif

typedef enum {
    CORRUPT_DATA    = -2,
    CORRUPT_STACK   = -1,
    STACK_OK        = 0,
    STK_NULL        = 1,
    ELM_WIDTH_NULL  = 2,
    BASE_CAP_NULL   = 3,
    DATA_NULL       = 4,
    STACK_OVERFLOW  = 5,
    TOO_BIG         = 6,
    STACK_UNDERFLOW = 7,
    REALLOC_NULL    = 8
} stack_err_t;

static const size_t DEFAULT_CAP = 16;

stack_t* stack_new (size_t elm_width, size_t base_capacity = DEFAULT_CAP);
void stack_delete (stack_t* stk);

stack_err_t stack_err (stack_t* stk);

void stack_dump(stack_t* stk, code_position_t pos = {"unknown", 0, "unknown"});
void stack_assert (stack_t* stk, code_position_t pos = {"unknown", 0, "unknown"});

size_t stack_curr_size (stack_t* stk);
void stack_push (stack_t* stk, const void* value);
void stack_pop (stack_t* stk, void* dst);

#endif // STACK_H
