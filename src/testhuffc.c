#include <stdio.h>
#include <string.h>
#ifndef _NON_PINTOS
#include <syscall.h>
#else
#include "lib.h"
#endif

int
main (int argc, char *argv[])
{
  if (argc != 2)
    {
      printf ("Usage: testhuffc <test_filename>\n");
      return EXIT_FAILURE;
    }

  char cmd1[32];
#ifndef _NON_PINTOS
  snprintf (cmd1, 32, "huffc -z %s", argv[1]);
#else
  snprintf (cmd1, 32, "./huffc -z %s", argv[1]);
#endif
  int pid = exec (cmd1);

  if (wait (pid) == 1)
    {
      printf ("huffc: failed to compress\n");
      return EXIT_FAILURE;
    }

  char filename_cp[32];
  strlcpy (filename_cp, argv[1], strlen (argv[1]) + 1);

  char *save_ptr;
  char *token = strtok_r (filename_cp, ".", &save_ptr);

  char cmd2[32];
#ifndef _NON_PINTOS
  snprintf (cmd2, 32, "huffc -u %s.cmp", token);
#else
  snprintf (cmd2, 32, "./huffc -u %s.cmp", token);
#endif
  pid = exec (cmd2);

  if (wait (pid) == 1)
    {
      printf ("huffc: failed to decompress\n");
      return EXIT_FAILURE;
    }

  char filename_ucmp[32];
  snprintf (filename_ucmp, 32, "%s.ucmp", token);

  int fd1, fd2;

  if ((fd1 = open (argv[1])) == -1)
    {
      printf ("open: failed to open sample\n");
      return EXIT_FAILURE;
    }

  if ((fd2 = open (filename_ucmp)) == -1)
    {
      printf ("open: failed to open sample.ucmp\n");
      return EXIT_FAILURE;
    }

  for (;;)
    {
      char buffer1[1024];
      char buffer2[1024];
      int readbyte1 = read (fd1, buffer1, 1024);
      int readbyte2 = read (fd2, buffer2, 1024);
      if (readbyte1 == 0 && readbyte2 == 0)
        break;
      if (readbyte1 != readbyte2)
        {
          printf ("zip: file size mismatch\n");
          return EXIT_FAILURE;
        }
      for (int i = 0; i < readbyte1; i++)
        if (buffer1[i] != buffer2[i])
          {
            printf ("zip: file content mismatch\n");
            return EXIT_FAILURE;
          }
    }
  return EXIT_SUCCESS;
}
