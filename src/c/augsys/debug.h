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
#ifndef AUGSYS_DEBUG_H
#define AUGSYS_DEBUG_H

#include "augsys/config.h"

#if defined(_MSC_VER) && !defined(NDEBUG)

# define _CRTDBG_MAP_ALLOC

# include <malloc.h>
# include <stdlib.h>

# include <crtdbg.h>

#endif /* _MSC_VER && !NDEBUG */

#if !defined(_MSC_VER) || defined(NDEBUG)

# define AUG_INITLEAKDUMP() (void)0
# define AUG_DUMPLEAKS() (void)0

#else /* !_MSC_VER || NDEBUG */

AUGSYS_API void
aug_initleakdump_(void);

AUGSYS_API void
aug_dumpleaks_(void);

# define AUG_INITLEAKDUMP() aug_initleakdump_()
# define AUG_DUMPLEAKS() aug_dumpleaks_()

#endif /* !_MSC_VER || NDEBUG */

#endif /* AUGSYS_DEBUG_H */
