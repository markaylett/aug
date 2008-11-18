/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSERVPP_CONFIG_HPP
#define AUGSERVPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSERVPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGSERVPP_SHARED)
# define AUGSERVPP_API
#else // AUGSERVPP_SHARED
# if !defined(AUGSERVPP_BUILD)
#  define AUGSERVPP_API AUG_IMPORT
# else // AUGSERVPP_BUILD
#  define AUGSERVPP_API AUG_EXPORT
# endif // AUGSERVPP_BUILD
#endif // AUGSERVPP_SHARED

#endif // AUGSERVPP_CONFIG_HPP
