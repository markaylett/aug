/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>
#include <stdlib.h> /* exit() */

#define SOH_ "\01"

static const char TEST_[] =
"8=FIX.4.4" SOH_
"9=25" SOH_
"35=D" SOH_
"49=sender" SOH_
"56=target" SOH_
"10=069" SOH_;

static const size_t TESTLEN_ = sizeof(TEST_) - 1;
static const aug_fixver_t FIXVER_ = "FIX.4.4";

static size_t received_ = 0;

static void
handler_(aug_object* ob, const char* buf, size_t size)
{
    struct aug_fixfield_ field;
    struct aug_fixstd_ fixstd;

    if (TESTLEN_ != size) {
        fprintf(stderr, "invalid buffer size [%d], expected [%d]\n",
                (int)size, (int)TESTLEN_);
        exit(1);
    }

    if (0 != memcmp(TEST_, buf, size)) {
        fprintf(stderr, "unexpected buffer contents\n");
        exit(1);
    }

    if (AUG_ISFAIL(aug_checkfix(&fixstd, buf, size))) {
        aug_perrinfo(aug_tlx, "aug_checkfix() failed", NULL);
        exit(1);
    }

    if (0 != strcmp(fixstd.fixver_, FIXVER_)) {
        fprintf(stderr, "unexpected fix version\n");
        exit(1);
    }

    while (fixstd.size_) {

        aug_rsize rsize = aug_fixfield(&field, fixstd.body_, fixstd.size_);

        if (AUG_ISFAIL(rsize)) {
            aug_perrinfo(aug_tlx, "aug_fixfield() failed", NULL);
            exit(1);
        }

        aug_ctxinfo(aug_tlx, "tag: %u", field.tag_);
        fixstd.body_ += AUG_RESULT(rsize);
        fixstd.size_ -= AUG_RESULT(rsize);
    }

    ++received_;
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_fixstream_t stream;
    aug_sd sv[2];
    aug_stream* in;
    int i;

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    mpool = aug_getmpool(aug_tlx);
    stream = aug_createfixstream(mpool, 0, handler_, NULL);

    if (!stream) {
        aug_perrinfo(aug_tlx, "aug_createfixstream() failed", NULL);
        return 1;
    }

    if (AUG_ISFAIL(aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv))) {
        aug_perrinfo(aug_tlx, "aug_socketpair() failed", NULL);
        return 1;
    }

    in = aug_createsstream(mpool, sv[1]);
    aug_release(mpool);

    if (!in) {
        aug_perrinfo(aug_tlx, "aug_createsstream() failed", NULL);
        return 1;
    }

    for (i = 0; i < TESTLEN_; ++i) {

        if (AUG_ISFAIL(aug_swrite(sv[0], TEST_ + i, 1))) {
            aug_perrinfo(aug_tlx, "aug_write() failed", NULL);
            return 1;
        }

        if (AUG_ISFAIL(aug_readfix(stream, in, AUG_BUFSIZE))) {
            aug_perrinfo(aug_tlx, "aug_parsefix() failed", NULL);
            return 1;
        }
    }

    if (1 != received_) {
        fprintf(stderr, "unexpected message count: %d\n", (int)received_);
        return 1;
    }

    if (AUG_ISFAIL(aug_swrite(sv[0], TEST_, TESTLEN_))) {
        aug_perrinfo(aug_tlx, "aug_write() failed", NULL);
        return 1;
    }

    if (AUG_ISFAIL(aug_swrite(sv[0], TEST_, TESTLEN_))) {
        aug_perrinfo(aug_tlx, "aug_write() failed", NULL);
        return 1;
    }

    if (AUG_ISFAIL(aug_readfix(stream, in, AUG_BUFSIZE))) {
        aug_perrinfo(aug_tlx, "aug_parsefix() failed", NULL);
        return 1;
    }

    if (3 != received_) {
        fprintf(stderr, "unexpected message count: %d\n", (int)received_);
        return 1;
    }

    aug_release(in);
    aug_sclose(sv[0]);
    aug_sclose(sv[1]);
    aug_destroyfixstream(stream);
    return 0;
}
