/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>
#include <string.h>

#define VERNO_ 1

#define ACCEPT_
#define ACK_
#define CONFLICT_
#define ELECTION_
#define MASTERACK_
#define MASTERREQ_
#define MASTERUP_
#define QUIT_
#define REFUSE_
#define SLAVEUP_
#define STATUS_

struct header_ {
    uint8_t verno_;
    char group_[31];
    uint32_t seqno_;
    uint8_t type_;
    uint8_t family_; /* 2 = IPV4, 23 = IPV6 */
    uint16_t port_;
    uint8_t addr_[16];
};

struct status_ {
    uint8_t weight_;
    char name_[31];
    uint32_t start_;
    uint32_t last_;
    uint32_t maxc_;
    uint16_t status_;
    char text_[151];
};

static void
fixedcpy_(char* dst, const char* src, size_t size)
{
    size_t i = aug_strlcpy(dst, src, size) + 1;
	if (i < size)
		memset(dst + i, '\0', size - i);
}

static void
write_(char* dst, const struct header_* src)
{
    fixedcpy_(dst, src->group_, sizeof(src->group_));
}

int
main(int argc, char* argv[])
{
    struct header_ h;
    char test[255];

    if (!aug_autotlx())
        return 1;

    aug_ctxinfo(aug_tlx, "header=%d, status=%d", (int)sizeof(struct header_),
                (int)sizeof(struct status_));
    strcpy(h.group_, "test");
    write_(test, &h);
    return 0;
}
