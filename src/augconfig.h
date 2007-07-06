/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCONFIG_H
#define AUGCONFIG_H

#if HAVE_CONFIG_H
# include "auglocal.h"
#else /*!HAVE_CONFIG_H */

# if !defined(ENABLE_MULTICAST)
#  define ENABLE_MULTICAST 1
# endif /* !ENABLE_MULTICAST */

# if !defined(ENABLE_PYTHON)
#  define ENABLE_PYTHON 1
# endif /* !ENABLE_PYTHON */

/* #undef ENABLE_RUBY */

# if !defined(ENABLE_SSL)
#  define ENABLE_SSL 1
# endif /* !ENABLE_SSL */

# if !defined(ENABLE_THREADS)
#  define ENABLE_THREADS 1
# endif /* !ENABLE_THREADS */

#endif /* !HAVE_CONFIG_H */

#if defined(_MSC_VER)

# if !defined(HAVE_IPV6)
#  define HAVE_IPV6 1
# endif /* !HAVE_IPV6 */

# define __func__ __FUNCTION__
# define PACKAGE_BUGREPORT "mark@emantic.co.uk"

#endif /* _MSC_VER */

#if !defined(__cplusplus)
# define AUG_EXTERNC extern
#else /* __cplusplus */
# define AUG_EXTERNC extern "C"
# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW throw()
# endif /* !AUG_NOTHROW */
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUG_EXPORT
# define AUG_IMPORT
#else /* _WIN32 */
# define AUG_EXPORT __declspec(dllexport)
# define AUG_IMPORT __declspec(dllimport)
#endif /* _WIN32 */

#endif /* AUGCONFIG_H */
