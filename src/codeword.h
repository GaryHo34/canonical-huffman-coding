#ifndef CODEWORD_H
#define CODEWORD_H
#include <stdio.h>

#ifdef _NON_PINTOS
#include "lib.h"
#endif

#define BYTE_TO_BINARY_PATTERN                                                \
  "%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)                                                  \
  ((byte) & 0x80000000 ? '1' : '0'), ((byte) & 0x40000000 ? '1' : '0'),       \
      ((byte) & 0x20000000 ? '1' : '0'), ((byte) & 0x10000000 ? '1' : '0'),   \
      ((byte) & 0x8000000 ? '1' : '0'), ((byte) & 0x4000000 ? '1' : '0'),     \
      ((byte) & 0x2000000 ? '1' : '0'), ((byte) & 0x1000000 ? '1' : '0'),     \
      ((byte) & 0x800000 ? '1' : '0'), ((byte) & 0x400000 ? '1' : '0'),       \
      ((byte) & 0x200000 ? '1' : '0'), ((byte) & 0x100000 ? '1' : '0'),       \
      ((byte) & 0x80000 ? '1' : '0'), ((byte) & 0x40000 ? '1' : '0'),         \
      ((byte) & 0x20000 ? '1' : '0'), ((byte) & 0x10000 ? '1' : '0'),         \
      ((byte) & 0x8000 ? '1' : '0'), ((byte) & 0x4000 ? '1' : '0'),           \
      ((byte) & 0x2000 ? '1' : '0'), ((byte) & 0x1000 ? '1' : '0'),           \
      ((byte) & 0x800 ? '1' : '0'), ((byte) & 0x400 ? '1' : '0'),             \
      ((byte) & 0x200 ? '1' : '0'), ((byte) & 0x100 ? '1' : '0'),             \
      ((byte) & 0x80 ? '1' : '0'), ((byte) & 0x40 ? '1' : '0'),               \
      ((byte) & 0x20 ? '1' : '0'), ((byte) & 0x10 ? '1' : '0'),               \
      ((byte) & 0x08 ? '1' : '0'), ((byte) & 0x04 ? '1' : '0'),               \
      ((byte) & 0x02 ? '1' : '0'), ((byte) & 0x01 ? '1' : '0')

struct Codeword
{
  char ch;
  int length;
  int code[8];
};

void code_inc_one (int *code);
void code_clean (int *code);
void code_cp (int *from, int *to);
void code_shift_right (int *code, int digit);
void debug_dump_codeword (struct Codeword *cw);
bool codeword_match (struct Codeword *char_mapping, int *code, int length);
#endif // CODEWORD_H