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

static aug_bool init_ = AUG_FALSE;
static aug_tlskey_t tlskey_;

static struct tls_*
gettls_(void)
{
    return aug_gettlsvalue_(tlskey_);
}

static aug_status
inittls_(void)
{
    /* Lock held. */

    struct tls_* tls;
    if (!init_) {

        /* TLS key must be initialised on first call. */

        aug_status status = aug_createtlskey_(&tlskey_);
        if (status < 0)
            return status;

        init_ = AUG_TRUE;
        goto skip;
    }

    if (!(tls = gettls_())) {
    skip:
        /* First call on this thread. */

        if (!(tls = malloc(sizeof(struct tls_))))
            return AUG_FAILURE;

        tls->refs_ = 0;
        tls->ctx_ = NULL;
        aug_settlsvalue_(tlskey_, tls);
    }
    ++tls->refs_;
    return AUG_SUCCESS;
}

static void
termtls_(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (0 == --tls->refs_) {

        /* Free resources associated with thread. */

        aug_settlsvalue_(tlskey_, NULL);
        if (tls->ctx_) {
            aug_release(tls->ctx_);
            tls->ctx_ = NULL;
        }
        free(tls);
    }
}

#else /* !ENABLE_THREADS */

/* Single-threaded. */

static struct tls_ tls_ = { 0, NULL };

static struct tls_*
gettls_(void)
{
    return &tls_;
}

static aug_status
inittls_(void)
{
    ++tls_.refs_;
    return AUG_SUCCESS;
}

static void
termtls_(void)
{
    assert(0 < tls_.refs_);
    if (0 == --tls_.refs_ && tls_.ctx_) {
        aug_release(tls_.ctx_);
        tls_.ctx_ = NULL;
    }
}

#endif /* !ENABLE_THREADS */

AUGCTX_API aug_status
aug_init(void)
{
    aug_status status = aug_initlock_();
    if (0 <= status) {
        aug_lock();
        status = inittls_();
        aug_unlock();
    }
    return status;
}

AUGCTX_API aug_status
aug_initbasicctx(void)
{
    aug_status status = aug_init();
    if (status < 0)
        return status;

    if (!aug_tlx) {
        aug_ctx* ctx = aug_createbasicctx();
        aug_settlx(ctx);
        aug_release(ctx);
    }
    return AUG_SUCCESS;
}

AUGCTX_API void
aug_term(void)
{
    termtls_();
}

AUGCTX_API void
aug_settlx(aug_ctx* ctx)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    /* Retain before release - avoiding issues with self assignment. */
    if (ctx)
        aug_retain(ctx);
    if (tls->ctx_)
        aug_release(tls->ctx_);
    tls->ctx_ = ctx;
}

AUGCTX_API aug_ctx*
aug_gettlx(void)
{
    struct tls_* tls = gettls_();
    assert(0 < tls->refs_);
    if (tls->ctx_)
        aug_retain(tls->ctx_);
    return tls->ctx_;
}

AUGCTX_API aug_ctx*
aug_tlx_(void)
{
    return gettls_()->ctx_;
}
