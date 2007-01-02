/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRV_CONFIG_H
#define AUGSRV_CONFIG_H

#if !defined(__cplusplus)
# define AUGSRV_EXTERN extern
#else /* __cplusplus */
# define AUGSRV_EXTERN extern "C"
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUGSRV_EXPORT AUGSRV_EXTERN
# define AUGSRV_IMPORT AUGSRV_EXTERN
#else /* _WIN32 */
# define AUGSRV_EXPORT AUGSRV_EXTERN __declspec(dllexport)
# define AUGSRV_IMPORT AUGSRV_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

#if defined(DLL_EXPORT)
# define AUGSRV_SHARED
#endif /* DLL_EXPORT */

#if !defined(AUGSRV_SHARED)
# define AUGSRV_API AUGSRV_EXTERN
#else /* AUGSRV_SHARED */
# if !defined(AUGSRV_BUILD)
#  define AUGSRV_API AUGSRV_IMPORT
# else /* AUGSRV_BUILD */
#  define AUGSRV_API AUGSRV_EXPORT
# endif /* AUGSRV_BUILD */
#endif /* AUGSRV_SHARED */

#if HAVE_CONFIG_H
# ifndef AUGCONFIG_H
# define AUGCONFIG_H
#  include "augconfig.h"
# endif /* AUGCONFIG_H */
#endif /* HAVE_CONFIG_H */

#if defined(_MSC_VER)
# if !defined(AUGSRV_BUILD)
#  pragma comment(lib, "libaugsrv.lib")
# endif /* AUGSRV_BUILD */
# pragma comment(lib, "libaugutil.lib")
#endif /* _MSC_VER */

#endif /* AUGSRV_CONFIG_H */
