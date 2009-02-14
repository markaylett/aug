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
#ifndef AUGMAR_CONFIG_H
#define AUGMAR_CONFIG_H

/**
 * @file augmar/config.h
 *
 * Definitions of the storage class macros.
 */

#include "augconfig.h"

/**
 * Integration of configuration information set by Autoconf.
 */

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGMAR_SHARED
#endif /* DLL_EXPORT || _WINDLL */

/**
 * The API storage class definition.
 */

#if !defined(AUGMAR_SHARED)
# define AUGMAR_API AUG_EXTERNC
#else /* AUGMAR_SHARED */
# if !defined(AUGMAR_BUILD)
#  define AUGMAR_API AUG_EXTERNC AUG_IMPORT
# else /* AUGMAR_BUILD */
#  define AUGMAR_API AUG_EXTERNC AUG_EXPORT
# endif /* AUGMAR_BUILD */
#endif /* AUGMAR_SHARED */

#if defined(_MSC_VER)
# if !defined(AUGMAR_BUILD)
#  pragma comment(lib, "libaugmar.lib")
# endif /* AUGMAR_BUILD */
# pragma comment(lib, "libaugsys.lib")
#endif /* _MSC_VER */

#endif /* AUGMAR_CONFIG_H */
