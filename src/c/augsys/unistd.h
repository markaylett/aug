/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UNISTD_H
#define AUGSYS_UNISTD_H

#include "augsys/config.h"
#include "augsys/types.h"

#if !defined(_WIN32)
# include <unistd.h>
#else // _WIN32
# include <direct.h>
# include <io.h>
# define ftruncate _chsize
# define fsync _commit
#endif // _WIN32

AUGSYS_API int
aug_close(int fd);

AUGSYS_API ssize_t
aug_read(int fd, void* buf, size_t size);

AUGSYS_API ssize_t
aug_write(int fd, const void* buf, size_t size);

#endif /* AUGSYS_UNISTD_H */
