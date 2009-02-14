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
#ifndef AUGNET_CONFIG_H
#define AUGNET_CONFIG_H

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGNET_SHARED
#endif /* DLL_EXPORT || _WINDLL */

#if !defined(AUGNET_SHARED)
# define AUGNET_API AUG_EXTERNC
#else /* AUGNET_SHARED */
# ifndef AUGNET_BUILD
#  define AUGNET_API AUG_EXTERNC AUG_IMPORT
# else /* AUGNET_BUILD */
#  define AUGNET_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGNET_BUILD */
#endif /* AUGNET_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGNET_BUILD)
#  pragma comment(lib, "libaugnet.lib")
# endif /* AUGNET_BUILD */
# pragma comment(lib, "libaugutil.lib")
# pragma comment(lib, "libeay32MT.lib")
# pragma comment(lib, "ssleay32MT.lib")
#endif /* _MSC_VER */

#endif /* AUGNET_CONFIG_H */
