#include <stdio.h>
#include <string.h>
#ifndef _NON_PINTOS
#include <syscall.h>
#else
#include "lib.h"
#endif

static void create_output_path (char *new_filename, char *filename,
                                char *extension);

int
main (int argc, char *argv[])
{
  if (argc != 2)
    {
      printf ("Usage: testhuffc <test_filename>\n");
      return EXIT_FAILURE;
    }

  char cmd1[256];

#ifdef _NON_PINTOS
  snprintf (cmd1, strlen (argv[1]) + 12, "./huffc -c %s", argv[1]);
#else
  snprintf (cmd1, strlen (argv[1]) + 12, "huffc -c %s", argv[1]);
#endif

  int pid = exec (cmd1);

  if (wait (pid) == 1)
    {
      printf ("huffc: failed to compress\n");
      return EXIT_FAILURE;
    }

  char filename_cp1[256];
  strlcpy (filename_cp1, argv[1], strlen (argv[1]) + 1);

  char compress_file[256] = "";
  create_output_path (compress_file, filename_cp1, ".cmp");

  char cmd2[256];

#ifdef _NON_PINTOS
  snprintf (cmd2, strlen (compress_file) + 12, "./huffc -d %s", compress_file);
#else
  snprintf (cmd2, strlen (compress_file) + 12, "huffc -d %s", compress_file);
#endif

  pid = exec (cmd2);

  if (wait (pid) == 1)
    {
      printf ("huffc: failed to decompress\n");
      return EXIT_FAILURE;
    }

  char filename_cp2[256];
  strlcpy (filename_cp2, argv[1], strlen (argv[1]));

  char decompressed_file[256] = "";
  create_output_path (decompressed_file, filename_cp2, ".ucmp");


  printf ("huffc: comparing %s and %s\n", argv[1], decompressed_file);

  int fd1, fd2;

  if ((fd1 = open (argv[1])) == -1)
    {
      printf ("open: failed to open sample\n");
      return EXIT_FAILURE;
    }

  if ((fd2 = open (decompressed_file)) == -1)
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
          printf ("huffc: file size mismatch\n");
          return EXIT_FAILURE;
        }
      for (int i = 0; i < readbyte1; i++)
        if (buffer1[i] != buffer2[i])
          {
            printf ("huffc: file content mismatch\n");
            return EXIT_FAILURE;
          }
    }
  printf ("huffc: test passed\n");
  return EXIT_SUCCESS;
}

static void
create_output_path (char *new_filename, char *filename, char *extension)
{
  char *save_ptr;
  char *dir[10];
  char *token = strtok_r (filename, "/", &save_ptr);
  int dir_count = 0;
  while (token != NULL)
    {
      dir[dir_count++] = token;
      token = strtok_r (NULL, "/", &save_ptr);
    }

  token = strtok_r (dir[dir_count - 1], ".", &save_ptr);
#ifdef _NON_PINTOS
  for (int i = 0; i < dir_count - 1; i++)
    {
      strncat (new_filename, dir[i], +strlen (dir[i]) + 1);
      strncat (new_filename, "/", +2);
    }
  strncat (new_filename, token, +strlen (token) + 1);
  strncat (new_filename, extension, +strlen (extension) + 1);
#else
  for (int i = 0; i < dir_count - 1; i++)
    {
      strlcat (new_filename, dir[i],
               strlen (new_filename) + strlen (dir[i]) + 1);
      strlcat (new_filename, "/", strlen (new_filename) + 2);
    }
  strlcat (new_filename, token, strlen (new_filename) + strlen (token) + 1);
  strlcat (new_filename, extension,
           strlen (new_filename) + strlen (extension) + 1);
#endif
}