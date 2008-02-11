/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_UNISTD_H
#define AUGSYS_UNISTD_H

#include "augsys/config.h"
#include "augsys/types.h"

#if !defined(_WIN32)
# include <unistd.h>
#else /* _WIN32 */
# include <direct.h>
# include <io.h>
# include <process.h>
# if !defined(ftruncate)
#  define ftruncate _chsize
# endif /* ftruncate */
# if !defined(fsync)
#  define fsync _commit /* FlushFileBuffers() */
# endif /* fsync */
#endif /* _WIN32 */

AUGSYS_API int
aug_close(int fd);

AUGSYS_API int
aug_open(const char* path, int flags, ...);

AUGSYS_API int
aug_pipe(int fds[2]);

AUGSYS_API ssize_t
aug_read(int fd, void* buf, size_t size);

AUGSYS_API ssize_t
aug_write(int fd, const void* buf, size_t len);

AUGSYS_API void
aug_msleep(unsigned ms);

#endif /* AUGSYS_UNISTD_H */
