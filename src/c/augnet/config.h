/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
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

#ifdef DLL_EXPORT
# define AUGNET_SHARED
#endif /* DLL_EXPORT */

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

#endif /* AUGNET_CONFIG_H */


