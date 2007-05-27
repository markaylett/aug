/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRT_CONFIG_H
#define AUGRT_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGRT_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGRT_SHARED)
# define AUGRT_API AUG_EXTERNC
#else /* AUGRT_SHARED */
# if !defined(AUGRT_BUILD)
#  define AUGRT_API AUG_EXTERNC AUG_IMPORT
# else /* AUGRT_BUILD */
#  define AUGRT_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGRT_BUILD */
#endif /* AUGRT_SHARED */

#endif /* AUGRT_CONFIG_H */
