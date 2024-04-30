#ifndef _LIB_H
#define _LIB_H
#include <fcntl.h>
#include <inttypes.h>
#include <string.h>
#include <stdbool.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define create(filename, _)                                                   \
  (creat (filename, S_IRWXU | S_IRWXG | S_IRWXO) != -1)
#define open(filename) open (filename, O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO)
#define filesize(fd) lseek (fd, 0, SEEK_END)
#define seek(fd, offset) lseek (fd, offset, SEEK_SET)
#define exec(cmd) system (cmd)
#define strlcpy(dst, src, size) strncpy (dst, src, size)
#define wait(pid) waitpid (pid, NULL, 0)
//
#endif // _LIB_H