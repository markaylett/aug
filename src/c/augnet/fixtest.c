/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augsys.h"

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
handler_(const struct aug_var* arg, const char* buf, size_t size)
{
    ssize_t ret;
    struct aug_fixfield_ field;
    struct aug_fixstd_ fixstd;

    if (TESTLEN_ != size) {
        fprintf(stderr, "invalid buffer size '%d', expected '%d'\n",
                (int)size, (int)TESTLEN_);
        exit(1);
    }

    if (0 != memcmp(TEST_, buf, size)) {
        fprintf(stderr, "unexpected buffer contents\n");
        exit(1);
    }

    if (-1 == aug_checkfix(&fixstd, buf, size)) {
        aug_perrinfo(NULL, "aug_checkfix() failed");
        exit(1);
    }

    if (0 != strcmp(fixstd.fixver_, FIXVER_)) {
        fprintf(stderr, "unexpected fix version\n");
        exit(1);
    }

    while (fixstd.size_) {

        if (-1 == (ret = aug_fixfield(&field, fixstd.body_, fixstd.size_))) {
            aug_perrinfo(NULL, "aug_fixfield() failed");
            exit(1);
        }

        fixstd.body_ += ret;
        fixstd.size_ -= ret;
    }

    ++received_;
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;

    int i, sv[2];
    aug_fixstream_t stream;

    aug_atexitinit(&errinfo);

    if (!(stream = aug_createfixstream(0, handler_, NULL))) {
        aug_perrinfo(NULL, "aug_createfixstream() failed");
        return 1;
    }

    if (-1 == aug_socketpair(AF_UNIX, SOCK_STREAM, 0, sv)) {
        aug_perrinfo(NULL, "aug_socketpair() failed");
        return 1;
    }

    for (i = 0; i < TESTLEN_; ++i) {

        if (-1 == aug_write(sv[0], TEST_ + i, 1)) {
            aug_perrinfo(NULL, "aug_write() failed");
            return 1;
        }

        if (-1 == aug_readfix(stream, sv[1], 4096)) {
            aug_perrinfo(NULL, "aug_parsefix() failed");
            return 1;
        }
    }

    if (1 != received_) {
        fprintf(stderr, "unexpected message count: %d\n", (int)received_);
        return 1;
    }

    if (-1 == aug_write(sv[0], TEST_, TESTLEN_)) {
        aug_perrinfo(NULL, "aug_write() failed");
        return 1;
    }

    if (-1 == aug_write(sv[0], TEST_, TESTLEN_)) {
        aug_perrinfo(NULL, "aug_write() failed");
        return 1;
    }

    if (-1 == aug_readfix(stream, sv[1], 4096)) {
        aug_perrinfo(NULL, "aug_parsefix() failed");
        return 1;
    }

    if (3 != received_) {
        fprintf(stderr, "unexpected message count: %d\n", (int)received_);
        return 1;
    }

    aug_close(sv[0]);
    aug_close(sv[1]);
    aug_freefixstream(stream);
    return 0;
}
