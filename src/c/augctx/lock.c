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
#define AUGCTX_BUILD
#include "augctx/lock.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#if ENABLE_THREADS
# if !defined(_WIN32)
#  include "augctx/posix/lock.c"
# else /* _WIN32 */
#  include "augctx/win32/lock.c"
# endif /* _WIN32 */
#else /* !ENABLE_THREADS */

/* Single-threaded stubs. */

AUG_EXTERNC aug_bool
aug_initlock_(void)
{
    return AUG_TRUE;
}

AUGCTX_API void
aug_lock(void)
{
}

AUGCTX_API void
aug_unlock(void)
{
}

#endif /* !ENABLE_THREADS */
