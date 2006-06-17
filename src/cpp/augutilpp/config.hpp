/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CONFIG_HPP
#define AUGUTILPP_CONFIG_HPP

#if !defined(_WIN32)
# define AUGUTILPP_EXPORT
# define AUGUTILPP_IMPORT
#else // _WIN32
# define AUGUTILPP_EXPORT __declspec(dllexport)
# define AUGUTILPP_IMPORT __declspec(dllimport)
#endif // _WIN32

#if defined(DLL_EXPORT)
# define AUGUTILPP_SHARED
#endif // DLL_EXPORT

#if !defined(AUGUTILPP_SHARED)
# define AUGUTILPP_API
#else // AUGUTILPP_SHARED
# if !defined(AUGUTILPP_BUILD)
#  define AUGUTILPP_API AUGUTILPP_IMPORT
# else // AUGUTILPP_BUILD
#  define AUGUTILPP_API AUGUTILPP_EXPORT
# endif // AUGUTILPP_BUILD
#endif // AUGUTILPP_SHARED

#if !defined(NOTHROW)
# define NOTHROW throw()
#endif // !NOTHROW

#endif // AUGUTILPP_CONFIG_HPP
