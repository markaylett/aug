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
#ifndef AUGSYS_ENDIAN_H
#define AUGSYS_ENDIAN_H

/**
 * @file augsys/endian.h
 *
 * Functions related to byte ordering.
 */

#include "augsys/config.h"
#include "augsys/types.h"

AUGSYS_API uint16_t
aug_swap16(uint16_t i);

AUGSYS_API uint32_t
aug_swap32(uint32_t i);

AUGSYS_API uint64_t
aug_swap64(uint64_t i);

AUGSYS_API uint16_t
aug_ntoh16(uint16_t i);

AUGSYS_API uint32_t
aug_ntoh32(uint32_t i);

AUGSYS_API uint64_t
aug_ntoh64(uint64_t i);

AUGSYS_API uint16_t
aug_hton16(uint16_t i);

AUGSYS_API uint32_t
aug_hton32(uint32_t i);

AUGSYS_API uint64_t
aug_hton64(uint64_t i);

AUGSYS_API uint16_t
aug_decode16(const char* ptr);

AUGSYS_API uint32_t
aug_decode32(const char* ptr);

AUGSYS_API uint64_t
aug_decode64(const char* ptr);

AUGSYS_API void
aug_encode16(uint16_t i, char* ptr);

AUGSYS_API void
aug_encode32(uint32_t i, char* ptr);

AUGSYS_API void
aug_encode64(uint64_t i, char* ptr);

#endif /* AUGSYS_ENDIAN_H */
