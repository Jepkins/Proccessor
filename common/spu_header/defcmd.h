//                     MRI MI  RI  I
//                     |   |   |   |
// possible sequence:  0 0 0 0 0 0 0 0    --- lowest byte represents first argument
//                       |   |   |   |
//                       MR  M   R   MARK (incompatible with others)
//                           |
//                           invalid
// I - immediate, R - register, M - RAM
// Example: 0xEE - rvalues, 0xE4 - lvalues
// Max sequence length = sizeof(int)
//
// seqn - max number of sequences
//
/*      code  name     seqn   possible sequence  */
/*      |     |        |      |                  */
DEFCMD_(0xFF, unknown, 0,     0x00)
DEFCMD_(0xF0, sleep,   1,     0xEE)
DEFCMD_(0xF1, slpdif,  1,     0xEE)
DEFCMD_(0x00, hlt,     0,     0x00)
DEFCMD_(0x01, push,   10,     0xEE)
DEFCMD_(0x02, pop,    10,     0xE4)
DEFCMD_(0x03, mov,     1,   0xEEE4)
DEFCMD_(0x04, mst,     1, 0xEEEEE0)
DEFCMD_(0xA1, inv,     0,     0x00)
DEFCMD_(0xA2, dub,     0,     0x00)
DEFCMD_(0xA3, add,     0,     0x00)
DEFCMD_(0xA4, sub,     0,     0x00)
DEFCMD_(0xA5, mul,     0,     0x00)
DEFCMD_(0xA6, div,     0,     0x00)
DEFCMD_(0xA7, sqrt,    0,     0x00)
DEFCMD_(0xA8, sin,     0,     0x00)
DEFCMD_(0xA9, cos,     0,     0x00)
DEFCMD_(0xAA, sqr,     0,     0x00)
DEFCMD_(0xD1, jmp,     1,     0x01)
DEFCMD_(0xD2, ja,      1,     0x01)
DEFCMD_(0xD3, jae,     1,     0x01)
DEFCMD_(0xD4, jb,      1,     0x01)
DEFCMD_(0xD5, jbe,     1,     0x01)
DEFCMD_(0xD6, je,      1,     0x01)
DEFCMD_(0xD7, jne,     1,     0x01)
DEFCMD_(0xDA, call,    1,     0x01)
DEFCMD_(0xDB, ret,     0,     0x00)
DEFCMD_(0xE0, putcc,   100,   0xEE)
DEFCMD_(0xE1, in,      0,     0x00)
DEFCMD_(0xE2, out,     0,     0x00)
DEFCMD_(0xE3, dump,    0,     0x00)
DEFCMD_(0xE4, draw,    0,     0x00)
