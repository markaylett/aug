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
#define AUGSYS_BUILD
#include "augsys/endian.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <string.h> /* memcpy() */

AUGSYS_API uint16_t
aug_swap16(uint16_t i)
{
    return  (i & 0xff00) >> 8
        | (i & 0x00ff) << 8;
}

AUGSYS_API uint32_t
aug_swap32(uint32_t i)
{
    return (i & 0xff000000) >> 24
        | (i & 0x00ff0000) >> 8
        | (i & 0x0000ff00) << 8
        | (i & 0x000000ff) << 24;
}

AUGSYS_API uint64_t
aug_swap64(uint64_t i)
{
    union {
        uint64_t i64_;
        uint32_t i32_[2];
    } u;
    uint32_t tmp;

    /*
       u = ABCDEFGH
       tmp = DCBA
    */

    u.i64_ = i;
    tmp = aug_swap32(u.i32_[0]);

    /*
       u = HGFEEFGH
       tmp = DCBA
    */

    u.i32_[0] = aug_swap32(u.i32_[1]);

    /*
       u = HGFEDCBA
       tmp = DCBA
    */

    u.i32_[1] = tmp;
    return u.i64_;
}

#if WORDS_BIGENDIAN

AUGSYS_API uint16_t
aug_ntoh16(uint16_t i)
{
    return i;
}

AUGSYS_API uint32_t
aug_ntoh32(uint32_t i)
{
    return i;
}

AUGSYS_API uint64_t
aug_ntoh64(uint64_t i)
{
    return i;
}

AUGSYS_API uint16_t
aug_hton16(uint16_t i)
{
    return i;
}

AUGSYS_API uint32_t
aug_hton32(uint32_t i)
{
    return i;
}

AUGSYS_API uint64_t
aug_hton64(uint64_t i)
{
    return i;
}

#else /* !WORDS_BIGENDIAN */

AUGSYS_API uint16_t
aug_ntoh16(uint16_t i)
{
    return aug_swap16(i);
}

AUGSYS_API uint32_t
aug_ntoh32(uint32_t i)
{
    return aug_swap32(i);
}

AUGSYS_API uint64_t
aug_ntoh64(uint64_t i)
{
    return aug_swap64(i);
}

AUGSYS_API uint16_t
aug_hton16(uint16_t i)
{
    return aug_swap16(i);
}

AUGSYS_API uint32_t
aug_hton32(uint32_t i)
{
    return aug_swap32(i);
}

AUGSYS_API uint64_t
aug_hton64(uint64_t i)
{
    return aug_swap64(i);
}

#endif /* !WORDS_BIGENDIAN */

AUGSYS_API uint16_t
aug_decode16(const char* ptr)
{
    uint16_t i;
    memcpy(&i, ptr, sizeof(i));
    return aug_ntoh16(i);
}

AUGSYS_API uint32_t
aug_decode32(const char* ptr)
{
    uint32_t i;
    memcpy(&i, ptr, sizeof(i));
    return aug_ntoh32(i);
}

AUGSYS_API uint64_t
aug_decode64(const char* ptr)
{
    uint64_t i;
    memcpy(&i, ptr, sizeof(i));
    return aug_ntoh64(i);
}

AUGSYS_API void
aug_encode16(uint16_t i, char* ptr)
{
    i = aug_hton16(i);
    memcpy(ptr, &i, sizeof(i));
}

AUGSYS_API void
aug_encode32(uint32_t i, char* ptr)
{
    i = aug_hton32(i);
    memcpy(ptr, &i, sizeof(i));
}

AUGSYS_API void
aug_encode64(uint64_t i, char* ptr)
{
    i = aug_hton64(i);
    memcpy(ptr, &i, sizeof(i));
}
