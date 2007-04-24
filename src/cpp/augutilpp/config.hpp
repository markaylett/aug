/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_CONFIG_HPP
#define AUGUTILPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGUTILPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGUTILPP_SHARED)
# define AUGUTILPP_API
#else // AUGUTILPP_SHARED
# if !defined(AUGUTILPP_BUILD)
#  define AUGUTILPP_API AUG_IMPORT
# else // AUGUTILPP_BUILD
#  define AUGUTILPP_API AUG_EXPORT
# endif // AUGUTILPP_BUILD
#endif // AUGUTILPP_SHARED

#endif // AUGUTILPP_CONFIG_HPP
