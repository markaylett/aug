/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_DEBUG_H
#define AUGSYS_DEBUG_H

#include "augsys/config.h"

#if defined(_MSC_VER) && !defined(NDEBUG)

# include <malloc.h>
# include <stdlib.h>

# define _CRTDBG_MAP_ALLOC
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
