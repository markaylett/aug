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
#include <stdlib.h> /* random(), srandom() */

#if ENABLE_THREADS
# include <pthread.h>
#endif /* ENABLE_THREADS */

AUGSYS_API long
aug_rand(void)
{
    return (long)random();
}

AUGSYS_API void
aug_srand(unsigned seed)
{
    srandom(seed);
}

AUGSYS_API unsigned
aug_threadid(void)
{
#if ENABLE_THREADS
    return (unsigned)pthread_self();
#else /* !ENABLE_THREADS */
    return 0;
#endif /* !ENABLE_THREADS */
}
