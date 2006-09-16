/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/endian.h"

static const char rcsid[] = "$Id$";

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
#if !defined(__GNUC__)
    return (i & 0xff00000000000000) >> 56
        | (i & 0x00ff000000000000) >> 40
        | (i & 0x0000ff0000000000) >> 24
        | (i & 0x000000ff00000000) >> 8
        | (i & 0x00000000ff000000) << 8
        | (i & 0x0000000000ff0000) << 24
        | (i & 0x000000000000ff00) << 40
        | (i & 0x00000000000000ff) << 56;
#else /* __GNUC__ */
    return (i & 0xff00000000000000LL) >> 56
        | (i & 0x00ff000000000000LL) >> 40
        | (i & 0x0000ff0000000000LL) >> 24
        | (i & 0x000000ff00000000LL) >> 8
        | (i & 0x00000000ff000000LL) << 8
        | (i & 0x0000000000ff0000LL) << 24
        | (i & 0x000000000000ff00LL) << 40
        | (i & 0x00000000000000ffLL) << 56;
#endif /* __GNUC__ */
}

#if _BYTE_ORDER == _BIG_ENDIAN
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

#else /* _BYTE_ORDER == _LITTLE_ENDIAN */

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

#endif /* _BYTE_ORDER == _LITTLE_ENDIAN */

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
aug_encode16(char* ptr, uint16_t i)
{
    i = aug_hton16(i);
    memcpy(ptr, &i, sizeof(i));
}

AUGSYS_API void
aug_encode32(char* ptr, uint32_t i)
{
    i = aug_hton32(i);
    memcpy(ptr, &i, sizeof(i));
}

AUGSYS_API void
aug_encode64(char* ptr, uint64_t i)
{
    i = aug_hton64(i);
    memcpy(ptr, &i, sizeof(i));
}
