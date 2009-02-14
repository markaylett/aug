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
#ifndef AUGSYS_CONFIG_H
#define AUGSYS_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSYS_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGSYS_SHARED)
# define AUGSYS_API AUG_EXTERNC
#else /* AUGSYS_SHARED */
# if !defined(AUGSYS_BUILD)
#  define AUGSYS_API AUG_EXTERNC AUG_IMPORT
# else /* AUGSYS_BUILD */
#  define AUGSYS_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGSYS_BUILD */
#endif /* AUGSYS_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGSYS_BUILD)
#  pragma comment(lib, "libaugsys.lib")
# endif /* AUGSYS_BUILD */
# pragma comment(lib, "iphlpapi.lib")
#endif /* _MSC_VER */

#endif /* AUGSYS_CONFIG_H */
