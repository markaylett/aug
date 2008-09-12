/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TYPES_H
#define AUGSYS_TYPES_H

#include "augconfig.h"

#include <stdarg.h>
#include <stddef.h> /* size_t */
#include <sys/types.h>

#if !defined(_WIN32)
typedef int aug_fd;
typedef int aug_sd;
# define AUG_BADFD (-1)
# define AUG_BADSD (-1)
#else /* _WIN32 */
#include <winsock2.h>
typedef HANDLE aug_fd;
typedef SOCKET aug_sd;
# define AUG_BADFD INVALID_HANDLE_VALUE
# define AUG_BADSD INVALID_SOCKET
#endif /* _WIN32 */

typedef aug_sd aug_md;
#define AUG_BADMD AUG_BADSD

#endif /* AUGSYS_TYPES_H */
