/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_CONFIG_HPP
#define AUGSYSPP_CONFIG_HPP

#if !defined(_WIN32)
# define AUGSYSPP_EXPORT
# define AUGSYSPP_IMPORT
#else // _WIN32
# define AUGSYSPP_EXPORT __declspec(dllexport)
# define AUGSYSPP_IMPORT __declspec(dllimport)
#endif // _WIN32

#if defined(DLL_EXPORT)
# define AUGSYSPP_SHARED
#endif // DLL_EXPORT

#if !defined(AUGSYSPP_SHARED)
# define AUGSYSPP_API
#else // AUGSYSPP_SHARED
# if !defined(AUGSYSPP_BUILD)
#  define AUGSYSPP_API AUGSYSPP_IMPORT
# else // AUGSYSPP_BUILD
#  define AUGSYSPP_API AUGSYSPP_EXPORT
# endif // AUGSYSPP_BUILD
#endif // AUGSYSPP_SHARED

#if !defined(NOTHROW)
# define NOTHROW throw()
#endif // !NOTHROW

#endif // AUGSYSPP_CONFIG_HPP
