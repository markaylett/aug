/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_CONFIG_H
#define AUGSRV_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSRV_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGSRV_SHARED)
# define AUGSRV_API AUG_EXTERNC
#else /* AUGSRV_SHARED */
# if !defined(AUGSRV_BUILD)
#  define AUGSRV_API AUG_EXTERNC AUG_IMPORT
# else /* AUGSRV_BUILD */
#  define AUGSRV_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGSRV_BUILD */
#endif /* AUGSRV_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGSRV_BUILD)
#  pragma comment(lib, "libaugsrv.lib")
# endif /* AUGSRV_BUILD */
# pragma comment(lib, "libaugutil.lib")
#endif /* _MSC_VER */

#endif /* AUGSRV_CONFIG_H */
