/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONFIG_HPP
#define AUGNETPP_CONFIG_HPP

#if !defined(_WIN32)
# define AUGNETPP_EXPORT
# define AUGNETPP_IMPORT
#else // _WIN32
# define AUGNETPP_EXPORT __declspec(dllexport)
# define AUGNETPP_IMPORT __declspec(dllimport)
#endif // _WIN32

#if defined(DLL_EXPORT)
# define AUGNETPP_SHARED
#endif // DLL_EXPORT

#if !defined(AUGNETPP_SHARED)
# define AUGNETPP_API
#else // AUGNETPP_SHARED
# if !defined(AUGNETPP_BUILD)
#  define AUGNETPP_API AUGNETPP_IMPORT
# else // AUGNETPP_BUILD
#  define AUGNETPP_API AUGNETPP_EXPORT
# endif // AUGNETPP_BUILD
#endif // AUGNETPP_SHARED

#if !defined(NOTHROW)
# define NOTHROW throw()
#endif // !NOTHROW

#endif // AUGNETPP_CONFIG_HPP
