/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_CONFIG_H
#define AUGSYS_CONFIG_H

#if !defined(__cplusplus)
# define AUGSYS_EXTERN extern
#else /* __cplusplus */
# define AUGSYS_EXTERN extern "C"
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUGSYS_EXPORT AUGSYS_EXTERN
# define AUGSYS_IMPORT AUGSYS_EXTERN
#else /* _WIN32 */
# define AUGSYS_EXPORT AUGSYS_EXTERN __declspec(dllexport)
# define AUGSYS_IMPORT AUGSYS_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

#if defined(DLL_EXPORT)
# define AUGSYS_SHARED
#endif /* DLL_EXPORT */

#if !defined(AUGSYS_SHARED)
# define AUGSYS_API AUGSYS_EXTERN
#else /* AUGSYS_SHARED */
# if !defined(AUGSYS_BUILD)
#  define AUGSYS_API AUGSYS_IMPORT
# else /* AUGSYS_BUILD */
#  define AUGSYS_API AUGSYS_EXPORT
# endif /* AUGSYS_BUILD */
#endif /* AUGSYS_SHARED */

#endif /* AUGSYS_CONFIG_H */
