/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#define AUGSYS_BUILD
#include "augsys/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/lock.h"

#include <limits.h>

#if !defined(_WIN32)
# include "augsys/posix/utility.c"
#else /* _WIN32 */
# include "augsys/win32/utility.c"
#endif /* _WIN32 */

AUGSYS_API void*
aug_memfrob(void* dst, size_t size)
{
    char* ptr = (char*)dst;
    while (size)
        ptr[--size] ^= 42;
    return dst;
}

AUGSYS_API unsigned
aug_nextid(void)
{
    static unsigned id_ = 1;
    unsigned id;

    aug_lock();
    if (id_ == INT_MAX) {
        id_ = 1;
        id = INT_MAX;
    } else
        id = id_++;
    aug_unlock();

    return id;
}
