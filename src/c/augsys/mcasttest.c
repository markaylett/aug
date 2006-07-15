/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#include <stdio.h>
#include <string.h>

struct aug_heartbeat {

    uint8_t magic_;
    uint8_t verno_;
    char group_[32];
    char name_[32];
    uint8_t weight_;
    uint8_t family_; /* 2 = IPV4, 23 = IPV6 */
    uint16_t port_;
    union {
        uint32_t ipv4_;
        uint8_t ipv6_[16];
    } addr_;
    uint16_t status_;
    uint64_t start_;
    uint64_t last_;
    uint64_t seqno_;
    uint32_t conns_;
    char text_[64];
};

static void
fixedcpy_(char* dst, const char* src, size_t size)
{
    size_t i = aug_strlcpy(dst, src, size) + 1;
	if (i < size)
		memset(dst + i, '\0', size - i);
}

#if 0
static void
write_(char* ptr, const struct aug_heartbeat* hb)
{
    fixedcpy_(ptr, hb->group_, sizeof(hb->group_));
}
#endif

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    char test[32];
    aug_atexitinit(&errinfo);
    fixedcpy_(test, "test", sizeof(test));
    return 0;
}
