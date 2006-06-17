/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/format_.h"

#if !defined(_WIN32)

# include <string.h>

# include <netinet/in.h>

# define ntohs_ ntohs
# define ntohl_ ntohl
# define htons_ htons
# define htonl_ htonl

#else /* _WIN32 */

/* The _X86_ macro is defined in the windows header. */

# include "augsys/windows.h"

# if !defined(_X86_)

#  include <winsock.h>

#  if defined(_MSC_VER)
#   pragma comment(lib, "ws2_32.lib")
#  endif /* _MSC_VER */

#  define ntohs_ ntohs
#  define ntohl_ ntohl
#  define htons_ htons
#  define htonl_ htonl

# else /* _X86_ */

/* Avoid linking to the winsock library where little endian is known. */

static aug_uint16_t
swap16_(aug_uint16_t i)
{
    return  (i & 0xff00) >> 8
        | (i & 0x00ff) << 8;
}

static aug_uint32_t
swap32_(aug_uint32_t i)
{
    return (i & 0xff000000) >> 24
        | (i & 0x00ff0000) >> 8
        | (i & 0x0000ff00) << 8
        | (i & 0x000000ff) << 24;
}

#  define ntohs_ swap16_
#  define ntohl_ swap32_
#  define htons_ swap16_
#  define htonl_ swap32_

# endif /* _X86_ */
#endif /* _WIN32 */

AUGMAR_EXTERN aug_uint16_t
aug_decode16_(const aug_byte_t* ptr)
{
    aug_uint16_t i;
    memcpy(&i, ptr, sizeof(i));
    return ntohs_(i);
}

AUGMAR_EXTERN aug_uint32_t
aug_decode32_(const aug_byte_t* ptr)
{
    aug_uint32_t i;
    memcpy(&i, ptr, sizeof(i));
    return ntohl_(i);
}

AUGMAR_EXTERN void
aug_encode16_(aug_byte_t* ptr, aug_uint16_t i)
{
    i = htons_(i);
    memcpy(ptr, &i, sizeof(i));
}

AUGMAR_EXTERN void
aug_encode32_(aug_byte_t* ptr, aug_uint32_t i)
{
    i = htonl_(i);
    memcpy(ptr, &i, sizeof(i));
}
