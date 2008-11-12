/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGMARPP_CONFIG_HPP
#define AUGMARPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGMARPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGMARPP_SHARED)
# define AUGMARPP_API
#else // AUGMARPP_SHARED
# if !defined(AUGMARPP_BUILD)
#  define AUGMARPP_API AUG_IMPORT
# else // AUGMARPP_BUILD
#  define AUGMARPP_API AUG_EXPORT
# endif // AUGMARPP_BUILD
#endif // AUGMARPP_SHARED

#endif // AUGMARPP_CONFIG_HPP
