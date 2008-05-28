/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTX_CONFIG_H
#define AUGCTX_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGCTX_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGCTX_SHARED)
# define AUGCTX_API AUG_EXTERNC
#else /* AUGCTX_SHARED */
# if !defined(AUGCTX_BUILD)
#  define AUGCTX_API AUG_EXTERNC AUG_IMPORT
# else /* AUGCTX_BUILD */
#  define AUGCTX_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGCTX_BUILD */
#endif /* AUGCTX_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGCTX_BUILD)
#  pragma comment(lib, "libaugctx.lib")
# endif /* AUGCTX_BUILD */
# pragma comment(lib, "ws2_32.lib")
#endif /* _MSC_VER */

#endif /* AUGCTX_CONFIG_H */
