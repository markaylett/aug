/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGMARPP_CONFIG_HPP
#define AUGMARPP_CONFIG_HPP

#if !defined(_WIN32)
# define AUGMARPP_EXPORT
# define AUGMARPP_IMPORT
#else // _WIN32
# define AUGMARPP_EXPORT __declspec(dllexport)
# define AUGMARPP_IMPORT __declspec(dllimport)
#endif // _WIN32

#if defined(DLL_EXPORT)
# define AUGMARPP_SHARED
#endif // DLL_EXPORT

#if !defined(AUGMARPP_SHARED)
# define AUGMARPP_API
#else // AUGMARPP_SHARED
# if !defined(AUGMARPP_BUILD)
#  define AUGMARPP_API AUGMARPP_IMPORT
# else // AUGMARPP_BUILD
#  define AUGMARPP_API AUGMARPP_EXPORT
# endif // AUGMARPP_BUILD
#endif // AUGMARPP_SHARED

#if !defined(AUG_NOTHROW)
# define AUG_NOTHROW throw()
#endif // !AUG_NOTHROW

#endif // AUGMARPP_CONFIG_HPP
