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
#define AUGCTX_BUILD
#include "augctx/base.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/lock.h"
#include "augctx/mpool.h"
#include "augctx/tls_.h"
#include "augctx/utility.h" /* aug_check() */

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>         /* abort(), malloc() */

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
        goto first;
    }

    if (!(tls = gettls_())) {
    first:
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
    assert(tls && 0 < tls->refs_);
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
        return -1;
    aug_settlx(ctx);
    aug_release(ctx);
    return 0;
}

AUGCTX_API aug_ctx*
aug_gettlx(void)
{
    struct tls_* tls = gettls_();
    if (!tls) {
        /* Lazy. */
        aug_check(aug_autotlx());
        tls = gettls_();
        aug_check(tls);
    }
    assert(0 < tls->refs_);
    if (tls->ctx_)
        aug_retain(tls->ctx_);
    return tls->ctx_;
}

AUGCTX_API aug_ctx*
aug_tlx_(void)
{
    struct tls_* tls = gettls_();
    if (!tls) {
        /* Lazy. */
        aug_check(aug_autotlx());
        tls = gettls_();
        aug_check(tls);
    }
    assert(0 < tls->refs_);
    return tls->ctx_;
}

AUGCTX_API struct aug_errinfo*
aug_tlerr_(void)
{
    struct tls_* tls = gettls_();
    if (!tls) {
        /* Lazy. */
        aug_check(AUG_TRUE == aug_autotlx());
        tls = gettls_();
        aug_check(tls);
    }
    assert(0 < tls->refs_);
    return aug_geterrinfo(tls->ctx_);
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

            if (aug_setbasictlx(mpool) < 0)
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
