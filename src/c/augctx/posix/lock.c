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
#include "augctx/utility.h" /* aug_check() */
#include <stdio.h>          /* fprintf() */
#include <stdlib.h>         /* abort() */
#include <pthread.h>

static pthread_mutex_t mutex_ = PTHREAD_MUTEX_INITIALIZER;

AUG_EXTERNC aug_bool
aug_initlock_(void)
{
    /* Mutex is initialised statically. */

    return AUG_TRUE;
}

AUGCTX_API void
aug_lock(void)
{
    aug_check(0 == pthread_mutex_lock(&mutex_));
}

AUGCTX_API void
aug_unlock(void)
{
    aug_check(0 == pthread_mutex_unlock(&mutex_));
}
