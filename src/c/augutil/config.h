/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_CONFIG_H
#define AUGUTIL_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGUTIL_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGUTIL_SHARED)
# define AUGUTIL_API AUG_EXTERNC
#else /* AUGUTIL_SHARED */
# ifndef AUGUTIL_BUILD
#  define AUGUTIL_API AUG_EXTERNC AUG_IMPORT
# else /* AUGUTIL_BUILD */
#  define AUGUTIL_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGUTIL_BUILD */
#endif /* AUGUTIL_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGUTIL_BUILD)
#  pragma comment(lib, "libaugutil.lib")
# endif /* AUGUTIL_BUILD */
# pragma comment(lib, "libaugsys.lib")
#endif /* _MSC_VER */

#endif /* AUGUTIL_CONFIG_H */
