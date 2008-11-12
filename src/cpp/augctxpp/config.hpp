/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGCTXPP_CONFIG_HPP
#define AUGCTXPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGCTXPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGCTXPP_SHARED)
# define AUGCTXPP_API
#else // AUGCTXPP_SHARED
# if !defined(AUGCTXPP_BUILD)
#  define AUGCTXPP_API AUG_IMPORT
# else // AUGCTXPP_BUILD
#  define AUGCTXPP_API AUG_EXPORT
# endif // AUGCTXPP_BUILD
#endif // AUGCTXPP_SHARED

#endif // AUGCTXPP_CONFIG_HPP
