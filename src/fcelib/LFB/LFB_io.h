/*
  LFB_io.h
  Copyright (C) 2021 Benjamin Futasz

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.
 */

/*
  LFB_mmap() - Simple mmap without offset
  LFB_munmap() - munmap
 */

#ifndef LFB_IO_H
#define LFB_IO_H

#ifndef _WIN32
#include <fcntl.h>     /* open */
#include <sys/mman.h>  /* mmap */
#include <sys/stat.h>  /* fstat */
#include <unistd.h>    /* close */
#endif

#ifdef __cplusplus
extern "C" {
#endif

/* Returns NULL on failure.

   Linux: Simple mmap without offset. Updates '*fsize' with filesize. */
void *LFB_mmap(size_t *length, const char *path, int prot, int flags)
{
#ifndef _WIN32
  void *addr;
  int fd;
  struct stat sb;
  int flags_open = 0;

  if (prot == (PROT_READ | PROT_WRITE))
    flags_open = O_RDWR;
  else if (prot == PROT_READ)
    flags_open = O_RDONLY;
  else if (prot == PROT_WRITE)
    flags_open = O_WRONLY;

  if ((fd = open(path, flags_open)) < 0)
    return NULL;

  if (fstat(fd, &sb) < 0)
    return NULL;

  *length = (size_t)sb.st_size;

  addr = mmap(NULL, *length, prot, flags, fd, (off_t)0);
  if (addr == MAP_FAILED)
    addr = NULL;

  close(fd);
  return addr;
#endif
}

/* Returns {-1, 0} */
int LFB_munmap(void *addr, size_t length)
{
#ifndef _WIN32
  return munmap(addr, length);
#endif
}

#ifdef __cplusplus
}  /* extern "C" */
#endif

#endif  /* LFB_IO_H */