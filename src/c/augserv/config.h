/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERV_CONFIG_H
#define AUGSERV_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSERV_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGSERV_SHARED)
# define AUGSERV_API AUG_EXTERNC
#else /* AUGSERV_SHARED */
# if !defined(AUGSERV_BUILD)
#  define AUGSERV_API AUG_EXTERNC AUG_IMPORT
# else /* AUGSERV_BUILD */
#  define AUGSERV_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGSERV_BUILD */
#endif /* AUGSERV_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGSERV_BUILD)
#  pragma comment(lib, "libaugserv.lib")
# endif /* AUGSERV_BUILD */
# pragma comment(lib, "libaugutil.lib")
#endif /* _MSC_VER */

#endif /* AUGSERV_CONFIG_H */
