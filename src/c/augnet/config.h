/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_CONFIG_H
#define AUGNET_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGNET_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGNET_SHARED)
# define AUGNET_API AUG_EXTERN
#else /* AUGNET_SHARED */
# ifndef AUGNET_BUILD
#  define AUGNET_API AUG_IMPORT
# else /* AUGNET_BUILD */
#  define AUGNET_API AUG_EXPORT
# endif /* AUGNET_BUILD */
#endif /* AUGNET_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGNET_BUILD)
#  pragma comment(lib, "libaugnet.lib")
# endif /* AUGNET_BUILD */
# pragma comment(lib, "libaugutil.lib")
# pragma comment(lib, "libeay32MT.lib")
# pragma comment(lib, "ssleay32MT.lib")
#endif /* _MSC_VER */

#endif /* AUGNET_CONFIG_H */
