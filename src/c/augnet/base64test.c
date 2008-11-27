/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augnet.h"
#include "augsys.h"
#include "augctx.h"

#include <stdio.h>

static char encoded_[1024];
static char decoded_[1024];

static aug_result
encode_(aug_object* ob, const char* buf, size_t len)
{
    strcat(encoded_, buf);
    return AUG_SUCCESS;
}

static aug_result
decode_(aug_object* ob, const char* buf, size_t len)
{
    strcat(decoded_, buf);
    return AUG_SUCCESS;
}

static aug_result
testencode_(aug_base64_t encoder, const char* in, const char* out)
{
    encoded_[0] = '\0';
    if (AUG_ISFAIL(aug_appendbase64(encoder, in, strlen(in)))) {
        aug_perrinfo(aug_tlx, "aug_appendbase64() failed", NULL);
        return AUG_FAILERROR;
    }
    if (AUG_ISFAIL(aug_finishbase64(encoder))) {
        aug_perrinfo(aug_tlx, "aug_endbase64() failed", NULL);
        return AUG_FAILERROR;
    }
    if (0 != strcmp(encoded_, out)) {
        fprintf(stderr, "unexpected encoding: %s\n", encoded_);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

static aug_result
testdecode_(aug_base64_t decoder, const char* in, const char* out)
{
    decoded_[0] = '\0';
    if (AUG_ISFAIL(aug_appendbase64(decoder, in, strlen(in)))) {
        aug_perrinfo(aug_tlx, "aug_appendbase64() failed", NULL);
        return AUG_FAILERROR;
    }
    if (AUG_ISFAIL(aug_finishbase64(decoder))) {
        aug_perrinfo(aug_tlx, "aug_endbase64() failed", NULL);
        return AUG_FAILERROR;
    }
    if (0 != strcmp(decoded_, out)) {
        fprintf(stderr, "unexpected decoding: %s\n", decoded_);
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS;
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_base64_t encoder, decoder;

    if (!aug_autodltlx())
        return 1;

    mpool = aug_getmpool(aug_tlx);
    encoder = aug_createbase64(mpool, AUG_ENCODE64, encode_, NULL);
    decoder = aug_createbase64(mpool, AUG_DECODE64, decode_, NULL);
    aug_release(mpool);

    if (!encoder) {
        aug_perrinfo(aug_tlx, "aug_createbase64() failed", NULL);
        return 1;
    }

    if (!decoder) {
        aug_perrinfo(aug_tlx, "aug_createbase64() failed", NULL);
        goto fail1;
    }

    if (AUG_ISFAIL(testencode_(encoder, "apples", "YXBwbGVz")))
        goto fail2;

    if (AUG_ISFAIL(testdecode_(decoder, "YXBwbGVz", "apples")))
        goto fail2;

    if (AUG_ISFAIL(testencode_(encoder, "oranges", "b3Jhbmdlcw==")))
        goto fail2;

    if (AUG_ISFAIL(testdecode_(decoder, "b3Jhbmdlcw==", "oranges")))
        goto fail2;

    if (AUG_ISFAIL(testencode_(encoder, "pears", "cGVhcnM=")))
        goto fail2;

    if (AUG_ISFAIL(testdecode_(decoder, "cGVhcnM=", "pears")))
        goto fail2;

    aug_destroybase64(decoder);
    aug_destroybase64(encoder);
    return 0;

 fail2:
    aug_destroybase64(decoder);

 fail1:
    aug_destroybase64(encoder);
    return 1;
}
