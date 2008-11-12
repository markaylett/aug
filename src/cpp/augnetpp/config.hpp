/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_CONFIG_HPP
#define AUGNETPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGNETPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGNETPP_SHARED)
# define AUGNETPP_API
#else // AUGNETPP_SHARED
# if !defined(AUGNETPP_BUILD)
#  define AUGNETPP_API AUG_IMPORT
# else // AUGNETPP_BUILD
#  define AUGNETPP_API AUG_EXPORT
# endif // AUGNETPP_BUILD
#endif // AUGNETPP_SHARED

#endif // AUGNETPP_CONFIG_HPP
