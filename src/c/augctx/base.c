/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/lock.h"
#include "augctx/tls_.h"

#include <assert.h>
#include <stdlib.h> /* malloc() */

struct tls_ {
    int refs_;
    aug_ctx* ctx_;
};

#if ENABLE_THREADS

# include "augctx/tls_.h"

static int init_ = 0;
static aug_tlskey_t tlskey_;

static struct tls_*
gettls_(void)
{
    return aug_gettlsvalue_(tlskey_);
}

static void
inittls_(void)
{
    struct tls_* tls;
    if (!init_) {
        aug_createtlskey_(&tlskey_);
        init_ = 1;
        goto skip;
    }
    if (!(tls = gettls_())) {
    skip:
        if (!(tls = malloc(sizeof(struct tls_))))
            abort();
        tls->refs_ = 0;
        tls->ctx_ = NULL;
        aug_settlsvalue_(tlskey_, tls);
    }
    ++tls->refs_;
}

static void
termtls_(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (0 == --tls->refs_) {
        aug_settlsvalue_(tlskey_, NULL);
        if (tls->ctx_) {
            aug_release(tls->ctx_);
            tls->ctx_ = NULL;
        }
        free(tls);
    }
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
    struct tls_* tls = gettls_();
    ++tls->refs_;
}

static void
termtls_(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (0 == --tls->refs_ && tls->ctx_) {
        aug_release(tls->ctx_);
        tls->ctx_ = NULL;
    }
}

#endif /* !ENABLE_THREADS */

AUGCTX_API void
aug_init(void)
{
    aug_initlock_();
    aug_lock();
    inittls_();
    aug_unlock();
}

AUGCTX_API void
aug_term(void)
{
    aug_lock();
    termtls_();
    aug_unlock();
}

AUGCTX_API void
aug_setctx(aug_ctx* ctx)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (tls->ctx_)
        aug_release(tls->ctx_);
    if (ctx)
        aug_retain(ctx);
    tls->ctx_ = ctx;
}

AUGCTX_API aug_ctx*
aug_getctx(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (tls->ctx_)
        aug_retain(tls->ctx_);
    return tls->ctx_;
}
