/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGASPP_CONFIG_HPP
#define AUGASPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGASPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGASPP_SHARED)
# define AUGASPP_API
#else // AUGASPP_SHARED
# if !defined(AUGASPP_BUILD)
#  define AUGASPP_API AUG_IMPORT
# else // AUGASPP_BUILD
#  define AUGASPP_API AUG_EXPORT
# endif // AUGASPP_BUILD
#endif // AUGASPP_SHARED

#endif // AUGASPP_CONFIG_HPP
