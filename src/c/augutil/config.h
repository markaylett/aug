/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGUTIL_CONFIG_H
#define AUGUTIL_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGUTIL_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGUTIL_SHARED)
# define AUGUTIL_API AUG_EXTERNC
#else /* AUGUTIL_SHARED */
# ifndef AUGUTIL_BUILD
#  define AUGUTIL_API AUG_EXTERNC AUG_IMPORT
# else /* AUGUTIL_BUILD */
#  define AUGUTIL_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGUTIL_BUILD */
#endif /* AUGUTIL_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGUTIL_BUILD)
#  pragma comment(lib, "libaugutil.lib")
# endif /* AUGUTIL_BUILD */
# pragma comment(lib, "libaugsys.lib")
#endif /* _MSC_VER */

#endif /* AUGUTIL_CONFIG_H */
