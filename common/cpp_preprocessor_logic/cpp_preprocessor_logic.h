#ifndef PREPROC_LOGIC
#define PREPROC_LOGIC

#define EMPTY()
#define DEFER(id) id EMPTY()
#define OBSTRUCT(id) id DEFER(EMPTY)()

#define EXPAND5(...) EXPAND4(EXPAND4(EXPAND4(__VA_ARGS__)))
#define EXPAND4(...) EXPAND3(EXPAND3(EXPAND3(__VA_ARGS__)))
#define EXPAND3(...) EXPAND2(EXPAND2(EXPAND2(__VA_ARGS__)))
#define EXPAND2(...) EXPAND1(EXPAND1(EXPAND1(__VA_ARGS__)))
#define EXPAND1(...) EXPAND(EXPAND(EXPAND(__VA_ARGS__)))
#define EXPAND(...) __VA_ARGS__

#define Q(...) #__VA_ARGS__
#define QUOTE(...) Q(__VA_ARGS__)

#define CAT(a, ...) PRIMITIVE_CAT(a, __VA_ARGS__)
#define PRIMITIVE_CAT(a, ...) a ## __VA_ARGS__

#define CHECK_N(x, n, ...) n
#define CHECK(...) CHECK_N(__VA_ARGS__, 0,)

#define NOT(x) CHECK(PRIMITIVE_CAT(NOT_, x))
#define NOT_0 ~, 1,

#define COMPL(b) PRIMITIVE_CAT(COMPL_, b)
#define COMPL_0 1
#define COMPL_1 0

#define BOOL(x) COMPL(NOT(x))

#define IIF(c) PRIMITIVE_CAT(IIF_, c)
#define IIF_0(t, ...) __VA_ARGS__
#define IIF_1(t, ...) t

#define IF(c) IIF(BOOL(c))

#define WHILE_NO_EXP(pred, op, ...)     \
    IF(pred(__VA_ARGS__))               \
    (                                   \
        OBSTRUCT(WHILE_INDIRECT)()      \
        (                               \
            pred, op, op(__VA_ARGS__)   \
        ),                              \
        __VA_ARGS__                     \
    )
#define WHILE_INDIRECT() WHILE_NO_EXP

#define WHILE(...) EXPAND3(WHILE_NO_EXP(__VA_ARGS__))

#define NOT_END(t, ...) NOT(CHECK(CAT(NOT_END_, t)))
#define NOT_END_TERMINATOR ~,1,
#define DELETE_FIRST_1(a, ...) __VA_ARGS__
#define DELETE_FIRST_2(a, ...) DELETE_FIRST_1(__VA_ARGS__)
#define DELETE_FIRST_3(a, ...) DELETE_FIRST_2(__VA_ARGS__)
#define DELETE_FIRST_4(a, ...) DELETE_FIRST_3(__VA_ARGS__)

// #define MINUS_MINUS(x) CHECK(PRIMITIVE_CAT(MINUS_MINUS_, x))
// #define MINUS_MINUS_1  ~,0,
// #define MINUS_MINUS_2  ~,1,
// #define MINUS_MINUS_3  ~,2,
// #define MINUS_MINUS_4  ~,3,
// #define MINUS_MINUS_5  ~,4,
// #define MINUS_MINUS_6  ~,5,
// #define MINUS_MINUS_7  ~,6,
// #define MINUS_MINUS_8  ~,7,
// #define MINUS_MINUS_9  ~,8,
// #define MINUS_MINUS_10 ~,9,
//
// #define DELETE_FIRST_TWO(a, b, ...) __VA_ARGS__
// #define DO_N_TIMES_OP(x, oper, ...) MINUS_MINUS(x), oper, oper(__VA_ARGS__)
// #define DO_N_TIMES_PRED(n, ...) BOOL(n)
// #define DO_N_TIMES(n, oper, ...) EXPAND(DEFER(DELETE_FIRST_TWO)(WHILE(DO_N_TIMES_PRED, DO_N_TIMES_OP, n, oper, __VA_ARGS__)))

#endif // PREPROC_LOGIC
