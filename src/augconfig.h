/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCONFIG_H
#define AUGCONFIG_H

#if HAVE_CONFIG_H
# include "auglocal.h"
#else /*!HAVE_CONFIG_H */

/* Select reasonable defaults when config is not available. */

# if !defined(ENABLE_MULTICAST)
#  define ENABLE_MULTICAST 1
# endif /* !ENABLE_MULTICAST */

# if !defined(ENABLE_PYTHON)
#  define ENABLE_PYTHON 1
# endif /* !ENABLE_PYTHON */

/* #undef ENABLE_RUBY */

# if !defined(ENABLE_SMP)
#  define ENABLE_SMP 1
# endif /* !ENABLE_SMP */

# if !defined(ENABLE_SSL)
#  define ENABLE_SSL 1
# endif /* !ENABLE_SSL */

# if !defined(ENABLE_STRICT)
#  define ENABLE_STRICT 0
# endif /* !ENABLE_STRICT */

# if !defined(ENABLE_THREADS)
#  define ENABLE_THREADS 1
# endif /* !ENABLE_THREADS */

#endif /* !HAVE_CONFIG_H */

#if !defined(_MSC_VER)
# include <inttypes.h>
#else /* _MSC_VER */

# if !defined(HAVE_IPV6)
#  define HAVE_IPV6 1
# endif /* !HAVE_IPV6 */

# define __func__ __FUNCTION__
# define PACKAGE_BUGREPORT "mark.aylett@gmail.com"

# if !defined(INT32_MIN)
#  define INT8_MIN   (-0x80)
#  define INT16_MIN  (-0x8000)
#  define INT32_MIN  (-0x80000000)
# endif /* !INT32_MIN */

# if !defined(INT32_MAX)
#  define INT8_MAX   0x7F
#  define INT16_MAX  0x7FFF
#  define INT32_MAX  0x7FFFFFFF
# endif /* !INT32_MAX */

# if !defined(UINT32_MAX)
#  define UINT8_MAX  0xFF
#  define UINT16_MAX 0xFFFF
#  define UINT32_MAX 0xFFFFFFFF
# endif /* !UINT32_MAX */

typedef int mode_t;
typedef int pid_t;
typedef int ssize_t;
typedef void* caddr_t;

typedef __int8 int8_t;
typedef unsigned __int8 uint8_t;

typedef __int16 int16_t;
typedef unsigned __int16 uint16_t;

typedef __int32 int32_t;
typedef unsigned __int32 uint32_t;

typedef __int64 int64_t;
typedef unsigned __int64 uint64_t;

#endif /* _MSC_VER */

#if !defined(__cplusplus)
# define AUG_EXTERNC extern
#else /* __cplusplus */
# define AUG_EXTERNC extern "C"
# if !defined(NDEBUG)
#  define AUG_NOTHROW throw()
# else /* NDEBUG */
#  define AUG_NOTHROW
# endif /* NDEBUG */
#endif /* __cplusplus */

#if defined(__CYGWIN__) || defined(__MINGW32__)
# define AUG_EXPORT __attribute__ ((dllexport))
# define AUG_IMPORT __attribute__ ((dllimport))
#elif defined(_MSC_VER)
# define AUG_EXPORT __declspec(dllexport)
# define AUG_IMPORT __declspec(dllimport)
#else /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */
# define AUG_EXPORT
# define AUG_IMPORT
#endif /* !__CYGWIN__ && !__MINGW__ && !__MSC_VER */

#endif /* AUGCONFIG_H */
