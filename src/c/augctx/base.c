/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGCTX_BUILD
#include "augctx/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/lock.h"
#include "augctx/mpool.h"
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

static aug_bool
inittls_(void)
{
    /* Lock held. */

    struct tls_* tls;
    if (!init_) {

        /* TLS key must be initialised on first call. */

        if (!aug_createtlskey_(&tlskey_))
            return AUG_FALSE;

        init_ = AUG_TRUE;
        goto skip;
    }

    if (!(tls = gettls_())) {
    skip:
        /* First call on this thread. */

        if (!(tls = malloc(sizeof(struct tls_))))
            return AUG_FALSE;

        tls->refs_ = 0;
        tls->ctx_ = NULL;
        aug_settlsvalue_(tlskey_, tls);
    }

    ++tls->refs_;
    return AUG_TRUE;
}

static void
termtls_(void)
{
    struct tls_* tls = gettls_();
    if (!tls)
        return;

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

static aug_bool
inittls_(void)
{
    ++tls_.refs_;
    return AUG_TRUE;
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

AUGCTX_API aug_bool
aug_init(void)
{
    aug_bool ok = aug_initlock_();
    if (ok) {
        aug_lock();
        ok = inittls_();
        aug_unlock();
    }
    return ok;
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

AUGCTX_API aug_result
aug_setbasictlx(aug_mpool* mpool)
{
    aug_ctx* ctx = aug_createbasicctx(mpool);
    if (!ctx)
        return AUG_FAILERROR;
    aug_settlx(ctx);
    aug_release(ctx);
    return AUG_SUCCESS;
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
    struct tls_* tls = gettls_();
    assert(tls && 0 < tls->refs_);
    return tls->ctx_;
}

AUGCTX_API struct aug_errinfo*
aug_tlerr_(void)
{
    return aug_geterrinfo(aug_tlx_());
}

AUGCTX_API aug_bool
aug_inittlx(void)
{
    aug_bool ok = aug_init();

    if (ok && !aug_tlx) {

        /* aug_init() succeeded, but thread-local context needs to be
           created. */

        aug_mpool* mpool = aug_createdlmalloc();

        if (mpool) {

            if (AUG_ISFAIL(aug_setbasictlx(mpool)))
                ok = AUG_FALSE;

            aug_release(mpool);

        } else {

            /* Failed to create mpool. */

            ok = AUG_FALSE;
        }

        if (!ok)
            aug_term();
    }

    return ok;
}

AUGCTX_API aug_bool
aug_autotlx(void)
{
    aug_bool ok = aug_inittlx();
    if (ok)
        atexit(aug_term);
    return ok;
}
