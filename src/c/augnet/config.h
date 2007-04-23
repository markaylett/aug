/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_CONFIG_H
#define AUGNET_CONFIG_H

#if !defined(__cplusplus)
# define AUGNET_EXTERN extern
#else /* __cplusplus */
# define AUGNET_EXTERN extern "C"
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUGNET_EXPORT AUGNET_EXTERN
# define AUGNET_IMPORT AUGNET_EXTERN
#else /* _WIN32 */
# define AUGNET_EXPORT AUGNET_EXTERN __declspec(dllexport)
# define AUGNET_IMPORT AUGNET_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGNET_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGNET_SHARED)
# define AUGNET_API AUGNET_EXTERN
#else /* AUGNET_SHARED */
# ifndef AUGNET_BUILD
#  define AUGNET_API AUGNET_IMPORT
# else /* AUGNET_BUILD */
#  define AUGNET_API AUGNET_EXPORT
# endif /* AUGNET_BUILD */
#endif /* AUGNET_SHARED */

#if HAVE_CONFIG_H
# ifndef AUGCONFIG_H
# define AUGCONFIG_H
#  include "augconfig.h"
# endif /* AUGCONFIG_H */
#endif /* HAVE_CONFIG_H */

#if defined(_MSC_VER)
# if !defined(AUGNET_BUILD)
#  pragma comment(lib, "libaugnet.lib")
# endif /* AUGNET_BUILD */
# pragma comment(lib, "libaugutil.lib")
# pragma comment(lib, "libeay32MT.lib")
# pragma comment(lib, "ssleay32MT.lib")
#endif /* _MSC_VER */

#endif /* AUGNET_CONFIG_H */
