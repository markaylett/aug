/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_CONFIG_HPP
#define AUGSRVPP_CONFIG_HPP

#if !defined(_WIN32)
# define AUGSRVPP_EXPORT
# define AUGSRVPP_IMPORT
#else // _WIN32
# define AUGSRVPP_EXPORT __declspec(dllexport)
# define AUGSRVPP_IMPORT __declspec(dllimport)
#endif // _WIN32

#if defined(DLL_EXPORT)
# define AUGSRVPP_SHARED
#endif // DLL_EXPORT

#if !defined(AUGSRVPP_SHARED)
# define AUGSRVPP_API
#else // AUGSRVPP_SHARED
# if !defined(AUGSRVPP_BUILD)
#  define AUGSRVPP_API AUGSRVPP_IMPORT
# else // AUGSRVPP_BUILD
#  define AUGSRVPP_API AUGSRVPP_EXPORT
# endif // AUGSRVPP_BUILD
#endif // AUGSRVPP_SHARED

#if !defined(AUG_NOTHROW)
# define AUG_NOTHROW throw()
#endif // !AUG_NOTHROW

#endif // AUGSRVPP_CONFIG_HPP
