#include "codeword.h"
#include "limits.h"

void
code_inc_one (int *code)
{
  for (int i = 0; i < 8; i++)
    {
      if (code[i] == INT_MAX)
        {
          code[i] = 0;
          continue;
        }
      else
        {
          code[i]++;
          return;
        }
    }
}

void
code_clean (int *code)
{
  for (int i = 0; i < 8; i++)
    code[i] = 0;
}

void
code_cp (int *from, int *to)
{
  for (int i = 0; i < 8; i++)
    to[i] = from[i];
}

void
code_shift_left (int *code, int digit)
{
  if (digit == 0)
    return;
  for (int i = 7; i >= 1; i--)
    {
      // shift self k digits
      code[i] <<= digit;
      // now accommadate the k digits from prev slot
      int tmp = (code[i - 1] >> (32 - digit));
      code[i] += tmp;
    }
  code[0] <<= digit;
}

bool
codeword_match (struct Codeword *char_mapping, int *code, int word_length)
{
  if (char_mapping->length != word_length)
    return false;
  for (int i = 0; i < 8; i++)
    {
      if (char_mapping->code[i] != code[i])
        return false;
    }
  return true;
}

void
debug_dump_codeword (struct Codeword *cw)
{
  printf ("char: %c, word_length: %d, code: ", cw->ch, cw->length);
  for (int i = 0; i < 8; i++)
    {
      printf (BYTE_TO_BINARY_PATTERN "\n", BYTE_TO_BINARY (cw->code[i]));
    }
  printf ("\n");
}