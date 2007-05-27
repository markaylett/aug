/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGCONFIG_H
#define AUGCONFIG_H

#if HAVE_CONFIG_H
# include "auglocal.h"
#endif /* HAVE_CONFIG_H */

#if defined(_MSC_VER)

# if !defined(HAVE_IPV6)
#  define HAVE_IPV6 1
# endif /* !HAVE_IPV6 */

# if !defined(HAVE_OPENSSL_SSL_H)
#  define HAVE_OPENSSL_SSL_H 1
# endif /* !HAVE_OPENSSL_SSL_H */

# define __func__ __FUNCTION__
# define PACKAGE_BUGREPORT "mark@emantic.co.uk"

#endif /* _MSC_VER */

#if !defined(__cplusplus)
# define AUG_EXTERNC extern
#else /* __cplusplus */
# define AUG_EXTERNC extern "C"
# if !defined(AUG_NOTHROW)
#  define AUG_NOTHROW throw()
# endif // !AUG_NOTHROW
#endif /* __cplusplus */

#if !defined(_WIN32)
# define AUG_EXPORT
# define AUG_IMPORT
#else /* _WIN32 */
# define AUG_EXPORT __declspec(dllexport)
# define AUG_IMPORT __declspec(dllimport)
#endif /* _WIN32 */

#endif /* AUGCONFIG_H */
