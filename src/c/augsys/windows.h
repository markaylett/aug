/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_WINDOWS_H
#define AUGSYS_WINDOWS_H

#include "augsys/config.h"

#if defined(_WIN32)
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h>
# if !defined(bzero)
#  define bzero ZeroMemory
# endif /* !bzero */
#endif /* _WIN32 */

#endif /* AUGSYS_WINDOWS_H */
