/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRTPP_CONFIG_HPP
#define AUGRTPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGRTPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGRTPP_SHARED)
# define AUGRTPP_API
#else // AUGRTPP_SHARED
# if !defined(AUGRTPP_BUILD)
#  define AUGRTPP_API AUG_IMPORT
# else // AUGRTPP_BUILD
#  define AUGRTPP_API AUG_EXPORT
# endif // AUGRTPP_BUILD
#endif // AUGRTPP_SHARED

#endif // AUGRTPP_CONFIG_HPP
