/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

int
main(int argc, char* argv[])
{
    struct aug_netevent event;
    char buf[AUG_NETEVENT_SIZE];

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    event.proto_ = 1;
    strcpy(event.name_, "name");
    strcpy(event.addr_, "addr");
    event.type_ = 2;
    event.state_ = 3;
    event.seq_ = 4;
    event.hbsec_ = 5;
    event.weight_ = 6;
    event.load_ = 7;

    aug_packnetevent(buf, &event);
    memset(&event, 0, sizeof(event));
    aug_unpacknetevent(&event, buf);

    if (1 != event.proto_) {
        fprintf(stderr, "unexpected proto  [%d]\n", event.proto_);
        return 1;
    }

    if (0 != strcmp(event.name_, "name")) {
        fprintf(stderr, "unexpected name [%s]\n", event.name_);
        return 1;
    }

    if (0 != strcmp(event.addr_, "addr")) {
        fprintf(stderr, "unexpected addr [%s]\n", event.addr_);
        return 1;
    }

    if (2 != event.type_) {
        fprintf(stderr, "unexpected type  [%d]\n", event.type_);
        return 1;
    }

    if (3 != event.state_) {
        fprintf(stderr, "unexpected state  [%d]\n", event.state_);
        return 1;
    }

    if (4 != event.seq_) {
        fprintf(stderr, "unexpected seq  [%d]\n", event.seq_);
        return 1;
    }

    if (5 != event.hbsec_) {
        fprintf(stderr, "unexpected hbsec  [%d]\n", event.hbsec_);
        return 1;
    }

    if (6 != event.weight_) {
        fprintf(stderr, "unexpected weight  [%d]\n", event.weight_);
        return 1;
    }

    if (7 != event.load_) {
        fprintf(stderr, "unexpected load  [%d]\n", event.load_);
        return 1;
    }

    return 0;
}
