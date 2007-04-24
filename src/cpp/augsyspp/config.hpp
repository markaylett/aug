/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_CONFIG_HPP
#define AUGSYSPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSYSPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGSYSPP_SHARED)
# define AUGSYSPP_API
#else // AUGSYSPP_SHARED
# if !defined(AUGSYSPP_BUILD)
#  define AUGSYSPP_API AUG_IMPORT
# else // AUGSYSPP_BUILD
#  define AUGSYSPP_API AUG_EXPORT
# endif // AUGSYSPP_BUILD
#endif // AUGSYSPP_SHARED

#endif // AUGSYSPP_CONFIG_HPP
