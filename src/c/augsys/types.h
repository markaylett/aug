/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
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
