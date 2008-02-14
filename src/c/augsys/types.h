/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TYPES_H
#define AUGSYS_TYPES_H

#include "augconfig.h"

#include <stdarg.h>
#include <stddef.h> /* size_t */
#include <sys/types.h>

typedef unsigned aug_len_t;

#if !defined(_WIN32)
typedef int aug_fd;
#else /* _WIN32 */
typedef void* aug_fd;
#endif /* _WIN32 */
#define AUG_BADFD ((aug_fd)-1)

#endif /* AUGSYS_TYPES_H */
