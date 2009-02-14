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
#ifndef AUGSYSPP_CONFIG_HPP
#define AUGSYSPP_CONFIG_HPP

#include "augconfig.h"

#if defined(DLL_EXPORT) || defined(_WINDLL)
# define AUGSYSPP_SHARED
#endif // DLL_EXPORT || _WINDLL

#if !defined(AUGSYSPP_SHARED)
# define AUGSYSPP_API
#else // AUGSYSPP_SHARED
# if !defined(AUGSYSPP_BUILD)
#  define AUGSYSPP_API AUG_IMPORT
# else // AUGSYSPP_BUILD
#  define AUGSYSPP_API AUG_EXPORT
# endif // AUGSYSPP_BUILD
#endif // AUGSYSPP_SHARED

#endif // AUGSYSPP_CONFIG_HPP
