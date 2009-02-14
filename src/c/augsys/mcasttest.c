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
