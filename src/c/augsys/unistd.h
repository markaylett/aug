/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGSYS_UNISTD_H
#define AUGSYS_UNISTD_H

#include "augsys/config.h"
#include "augsys/types.h"

#include "augtypes.h"

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
#  define fsync _commit
# endif /* fsync */
#endif /* _WIN32 */

/**
 * Close file.
 *
 * @param fd File descriptor.
 *
 * @return See @ref TypesResult.
 */

AUGSYS_API aug_result
aug_fclose_I(aug_fd fd);

AUGSYS_API aug_result
aug_fsetnonblock_AI(aug_fd fd, aug_bool on);

AUGSYS_API aug_fd
aug_vfopen_N(const char* path, int flags, va_list args);

AUGSYS_API aug_fd
aug_fopen_N(const char* path, int flags, ...);

AUGSYS_API aug_result
aug_fpipe(aug_fd fds[2]);

AUGSYS_API aug_rsize
aug_fread_AI(aug_fd fd, void* buf, size_t size);

AUGSYS_API aug_rsize
aug_fwrite_AI(aug_fd fd, const void* buf, size_t size);

AUGSYS_API aug_result
aug_fsync(aug_fd fd);

/**
 * Similar to POSIX semantics except that gaps, in extended files, are not
 * guaranteed to be zero-filled.
 */

AUGSYS_API aug_result
aug_ftruncate_AI(aug_fd fd, off_t size);

AUGSYS_API aug_result
aug_fsize_IN(aug_fd fd, size_t* size);

AUGSYS_API void
aug_msleep_I(unsigned ms);

#endif /* AUGSYS_UNISTD_H */
