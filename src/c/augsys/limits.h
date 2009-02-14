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
#ifndef AUGSYS_LIMITS_H
#define AUGSYS_LIMITS_H

#include "augconfig.h"

#if !defined(_WIN32)
# include <limits.h>
# if !defined(PATH_MAX)
#  define AUG_PATH_MAX 255
# else /* PATH_MAX */
#  define AUG_PATH_MAX PATH_MAX
# endif /* PATH_MAX */
#else /* _WIN32 */
# include <stdlib.h>
# if !defined(_MAX_PATH)
#  define AUG_PATH_MAX 260
# else /* _MAX_PATH */
#  define AUG_PATH_MAX _MAX_PATH
# endif /* _MAX_PATH */
#endif /* _WIN32 */

#endif /* AUGSYS_LIMITS_H */
