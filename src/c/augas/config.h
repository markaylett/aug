/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_CONFIG_H
#define AUGAS_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGAS_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGAS_SHARED)
# define AUGAS_API AUG_EXTERNC
#else /* AUGAS_SHARED */
# if !defined(AUGAS_BUILD)
#  define AUGAS_API AUG_EXTERNC AUG_IMPORT
# else /* AUGAS_BUILD */
#  define AUGAS_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGAS_BUILD */
#endif /* AUGAS_SHARED */

#endif /* AUGAS_CONFIG_H */
