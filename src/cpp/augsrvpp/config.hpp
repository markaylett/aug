/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSRVPP_CONFIG_HPP
#define AUGSRVPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSRVPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGSRVPP_SHARED)
# define AUGSRVPP_API
#else // AUGSRVPP_SHARED
# if !defined(AUGSRVPP_BUILD)
#  define AUGSRVPP_API AUG_IMPORT
# else // AUGSRVPP_BUILD
#  define AUGSRVPP_API AUG_EXPORT
# endif // AUGSRVPP_BUILD
#endif // AUGSRVPP_SHARED

#endif // AUGSRVPP_CONFIG_HPP
