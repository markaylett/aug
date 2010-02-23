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
#ifndef AUGASPP_CONFIG_HPP
#define AUGASPP_CONFIG_HPP

#include "augconfig.h"

// Only c-libraries are shared.
#if !defined(__cplusplus)
 #if defined(DLL_EXPORT) || defined(_WINDLL)
 # define AUGASPP_SHARED
 #endif // DLL_EXPORT || _WINDLL
#endif // !__cplusplus

#if !defined(AUGASPP_SHARED)
# define AUGASPP_API
#else // AUGASPP_SHARED
# if !defined(AUGASPP_BUILD)
#  define AUGASPP_API AUG_IMPORT
# else // AUGASPP_BUILD
#  define AUGASPP_API AUG_EXPORT
# endif // AUGASPP_BUILD
#endif // AUGASPP_SHARED

#endif // AUGASPP_CONFIG_HPP
