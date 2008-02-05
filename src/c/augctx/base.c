/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/ctx.h"
#include "augctx/tls_.h"
struct tls_ {
    int refs_;
    aug_ctx* ctx_;
};

#if ENABLE_THREADS

# include "augsys/tls_.h"

static int init_ = 0;
static aug_tlskey_t tlskey_;

static struct tls_*
gettls_(void)
{
    return aug_gettlsvalue_(tlskey_, &tls);
}

static void
initkey_(void)
{
    if (!init_) {
        aug_createtlskey_(&tlskey_);
        init_ = 1;
    }
}

static void
inittls_(void)
{
    struct tls_* tls;
    initkey_();
    if (!(tls = gettls_())) {
        if (!(tls = malloc(sizeof(struct tls_))))
            abort();
        tls->refs_ = 0;
        tls->ctx_ = NULL;
        aug_settlsvalue_(tlskey_, tls);
    }
    ++tls->refs_;
}

#else /* !ENABLE_THREADS */

static struct tls_ tls_ = { 0, NULL };

static struct tls_*
gettls_(void)
{
    return &tls_;
}

static void
inittls_(void)
{
    ++tls->refs_;
}

#endif /* !ENABLE_THREADS */
