/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file config.h
   \brief Definitions of the storage class macros.
 */

#ifndef AUGMAR_CONFIG_H
#define AUGMAR_CONFIG_H

/**
   \brief The extern storage class definition.
 */

#if !defined(__cplusplus)
# define AUGMAR_EXTERN extern
#else /* __cplusplus */
# define AUGMAR_EXTERN extern "C"
#endif /* __cplusplus */

/**
   \brief The import and export storage class definitions.
 */

#if !defined(_WIN32)
# define AUGMAR_EXPORT AUGMAR_EXTERN
# define AUGMAR_IMPORT AUGMAR_EXTERN
#else /* _WIN32 */
# define AUGMAR_EXPORT AUGMAR_EXTERN __declspec(dllexport)
# define AUGMAR_IMPORT AUGMAR_EXTERN __declspec(dllimport)
#endif /* _WIN32 */

/**
   \brief Integration of configuration information set by Autoconf.
 */

#if defined(DLL_EXPORT)
# define AUGMAR_SHARED
#endif /* DLL_EXPORT */

/**
   \brief The API storage class definition.
 */

#if !defined(AUGMAR_SHARED)
# define AUGMAR_API AUGMAR_EXTERN
#else /* AUGMAR_SHARED */
# if !defined(AUGMAR_BUILD)
#  define AUGMAR_API AUGMAR_IMPORT
# else /* AUGMAR_BUILD */
#  define AUGMAR_API AUGMAR_EXPORT
# endif /* AUGMAR_BUILD */
#endif /* AUGMAR_SHARED */

#if HAVE_CONFIG_H
# ifndef AUGCONFIG_H
# define AUGCONFIG_H
#  include "augconfig.h"
# endif /* AUGCONFIG_H */
#endif /* HAVE_CONFIG_H */

#if defined(_MSC_VER)
# if !defined(AUGMAR_BUILD)
#  pragma comment(lib, "libaugmar.lib")
# endif /* AUGMAR_BUILD */
# pragma comment(lib, "libaugsys.lib")
#endif /* _MSC_VER */

#endif /* AUGMAR_CONFIG_H */
