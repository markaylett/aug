/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_CONFIG_H
#define AUGUTIL_CONFIG_H

#if !defined(__cplusplus)
# define AUGUTIL_EXTERN extern
#else /* __cplusplus */
# define AUGUTIL_EXTERN extern "C"
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUGUTIL_EXPORT AUGUTIL_EXTERN
# define AUGUTIL_IMPORT AUGUTIL_EXTERN
#else /* _WIN32 */
# define AUGUTIL_EXPORT AUGUTIL_EXTERN __declspec(dllexport)
# define AUGUTIL_IMPORT AUGUTIL_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

#ifdef DLL_EXPORT
# define AUGUTIL_SHARED
#endif /* DLL_EXPORT */

#if !defined(AUGUTIL_SHARED)
# define AUGUTIL_API AUGUTIL_EXTERN
#else /* AUGUTIL_SHARED */
# ifndef AUGUTIL_BUILD
#  define AUGUTIL_API AUGUTIL_IMPORT
# else /* AUGUTIL_BUILD */
#  define AUGUTIL_API AUGUTIL_EXPORT
# endif /* AUGUTIL_BUILD */
#endif /* AUGUTIL_SHARED */

#if HAVE_CONFIG_H
# ifndef AUGCONFIG_H
# define AUGCONFIG_H
#  include "augconfig.h"
# endif /* AUGCONFIG_H */
#endif /* HAVE_CONFIG_H */

#if defined(_MSC_VER)
# if !defined(AUGUTIL_BUILD)
#  pragma comment(lib, "libaugutil.lib")
# endif /* AUGUTIL_BUILD */
# pragma comment(lib, "libaugsys.lib")
#endif /* _MSC_VER */

#endif /* AUGUTIL_CONFIG_H */
