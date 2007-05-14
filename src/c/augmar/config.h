/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   Definitions of the storage class macros.

   \file config.h
 */
#ifndef AUGMAR_CONFIG_H
#define AUGMAR_CONFIG_H

#include "augconfig.h"

/**
   Integration of configuration information set by Autoconf.
 */

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGMAR_SHARED
#endif /* DLL_EXPORT || _WINDLL */

/**
   The API storage class definition.
 */

#if !defined(AUGMAR_SHARED)
# define AUGMAR_API AUG_EXTERN
#else /* AUGMAR_SHARED */
# if !defined(AUGMAR_BUILD)
#  define AUGMAR_API AUG_IMPORT
# else /* AUGMAR_BUILD */
#  define AUGMAR_API AUG_EXPORT
# endif /* AUGMAR_BUILD */
#endif /* AUGMAR_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGMAR_BUILD)
#  pragma comment(lib, "libaugmar.lib")
# endif /* AUGMAR_BUILD */
# pragma comment(lib, "libaugsys.lib")
#endif /* _MSC_VER */

#endif /* AUGMAR_CONFIG_H */
