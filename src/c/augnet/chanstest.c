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
#include "augnet.h"
#include "augctx.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h> /* exit() */

static int refs_ = 0;

struct impl_ {
    aug_chan chan_;
    aug_stream stream_;
    int refs_;
    aug_mpool* mpool_;
    aug_id id_;
};

static void*
cast_(struct impl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_chanid)) {
        aug_retain(&impl->chan_);
        return &impl->chan_;
    } else if (AUG_EQUALID(id, aug_streamid)) {
        aug_retain(&impl->stream_);
        return &impl->stream_;
    }
    return NULL;
}

static void
retain_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
    ++refs_;
}

static void
release_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        aug_freemem(mpool, impl);
        aug_release(mpool);
    }
    --refs_;
}

static void*
chan_cast_(aug_chan* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return cast_(impl, id);
}

static void
chan_retain_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    retain_(impl);
}

static void
chan_release_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    release_(impl);
}

static aug_result
chan_close_(aug_chan* ob)
{
    return AUG_SUCCESS;
}

static aug_chan*
chan_process_(aug_chan* ob, aug_chandler* handler, aug_bool* fork)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);

    /* Lock here to prevent release during callback. */

    retain_(impl);

    if (!aug_readychan(handler, &impl->chan_, 0)) {
        release_(impl);
        return NULL;
    }

    return ob;
}

static aug_result
chan_setwantwr_(aug_chan* ob, aug_bool wantwr)
{
    return AUG_SUCCESS;
}

static aug_id
chan_getid_(aug_chan* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, chan_, ob);
    return impl->id_;
}

static char*
chan_getname_(aug_chan* ob, char* dst, unsigned size)
{
    strcpy(dst, "test");
    return dst;
}

static aug_bool
chan_isready_(aug_chan* ob)
{
    return AUG_FALSE;
}

static const struct aug_chanvtbl chanvtbl_ = {
    chan_cast_,
    chan_retain_,
    chan_release_,
    chan_close_,
    chan_process_,
    chan_setwantwr_,
    chan_getid_,
    chan_getname_,
    chan_isready_
};

static void*
stream_cast_(aug_stream* ob, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    return cast_(impl, id);
}

static void
stream_retain_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    retain_(impl);
}

static void
stream_release_(aug_stream* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, stream_, ob);
    release_(impl);
}

static aug_result
stream_shutdown_(aug_stream* ob)
{
    return AUG_SUCCESS;
}

static aug_rsize
stream_read_(aug_stream* ob, void* buf, size_t size)
{
    return AUG_ZERO;
}

static aug_rsize
stream_readv_(aug_stream* ob, const struct iovec* iov, int size)
{
    return AUG_ZERO;
}

static aug_rsize
stream_write_(aug_stream* ob, const void* buf, size_t size)
{
    return AUG_ZERO;
}

static aug_rsize
stream_writev_(aug_stream* ob, const struct iovec* iov, int size)
{
    return AUG_ZERO;
}

static const struct aug_streamvtbl streamvtbl_ = {
    stream_cast_,
    stream_retain_,
    stream_release_,
    stream_shutdown_,
    stream_read_,
    stream_readv_,
    stream_write_,
    stream_writev_
};

static aug_chan*
create_(aug_mpool* mpool, aug_id id)
{
    struct impl_* impl = aug_allocmem(mpool, sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->chan_.vtbl_ = &chanvtbl_;
    impl->chan_.impl_ = NULL;
    impl->stream_.vtbl_ = &streamvtbl_;
    impl->stream_.impl_ = NULL;
    impl->refs_ = 1;
    impl->mpool_ = mpool;
    impl->id_ = id;

    ++refs_;

    aug_retain(mpool);
    return &impl->chan_;
}

static aug_bool (*cb_)(aug_chan*, unsigned short) = NULL;

static void*
chandler_cast_(aug_chandler* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_chandlerid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
chandler_retain_(aug_chandler* ob)
{
}

static void
chandler_release_(aug_chandler* ob)
{
}

static aug_bool
chandler_auth_(aug_chandler* ob, aug_id id, const char* subject,
               const char* issuer)
{
    return AUG_TRUE;
}

static void
chandler_clear_(aug_chandler* ob, aug_id id)
{
    aug_ctxinfo(aug_tlx, "clearing id: %d", (int)id);
}

static void
chandler_error_(aug_chandler* ob, aug_chan* chan,
                const struct aug_errinfo* errinfo)
{
    aug_perrinfo(aug_tlx, "socket error", errinfo);
}

static aug_bool
chandler_estab_(aug_chandler* ob, aug_chan* chan, aug_id parent)
{
    return AUG_TRUE;
}

static aug_bool
chandler_ready_(aug_chandler* ob, aug_chan* chan, unsigned short events)
{
    return cb_(chan, events);
}

static const struct aug_chandlervtbl chandlervtbl_ = {
    chandler_cast_,
    chandler_retain_,
    chandler_release_,
    chandler_auth_,
    chandler_clear_,
    chandler_error_,
    chandler_estab_,
    chandler_ready_
};

static aug_chandler chandler_ = { &chandlervtbl_, NULL };

static int count_ = 0;
static aug_id last_ = 0;

static aug_bool
stats_(aug_chan* chan, unsigned short events)
{
    const aug_id id = aug_getchanid(chan);
    if (3 < id)
        aug_die("invalid channel");
    ++count_;
    last_ = id;
    return AUG_TRUE;
}

static aug_bool
rm1_(aug_chan* chan, unsigned short events)
{
    const aug_id id = aug_getchanid(chan);
    return 1 != id;
}

static void
foreach_(aug_chans_t chans)
{
    cb_ = stats_;
    count_ = 0;
    last_ = 0;
    aug_processchans(chans);
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_chan* chan1;
    aug_chan* chan2;
    aug_chan* chan3;
    aug_chans_t chans;

    aug_check(aug_autotlx());
    aug_setloglevel(aug_tlx, AUG_LOGDEBUG0 + 3);

    mpool = aug_getmpool(aug_tlx);
    aug_check(mpool);

    chan1 = create_(mpool, 1);
    aug_check(chan1);

    chan2 = create_(mpool, 2);
    aug_check(chan2);

    chan3 = create_(mpool, 3);
    aug_check(chan3);

    chans = aug_createchans(mpool, &chandler_);
    aug_check(chans);

    aug_check(AUG_ISSUCCESS(aug_insertchan(chans, chan1)));
    aug_check(AUG_ISSUCCESS(aug_insertchan(chans, chan2)));
    aug_check(AUG_ISSUCCESS(aug_insertchan(chans, chan3)));
    aug_check(6 == refs_);

    aug_release(chan1);
    aug_release(chan2);
    aug_release(chan3);
    aug_check(3 == refs_);

    /* 3 2 1 */
    foreach_(chans);
    /* 2 1 3 */
    aug_check(3 == aug_getchans(chans));
    aug_check(3 == count_);
    aug_check(3 == last_);

    /* Fairness rotation. */

    /* 2 1 3 */
    foreach_(chans);
    /* 1 3 2 */
    aug_check(3 == aug_getchans(chans));
    aug_check(3 == count_);
    aug_check(2 == last_);

    /* Fairness rotation. */

    /* 1 3 2 */
    foreach_(chans);
    /* 3 2 1 */
    aug_check(3 == aug_getchans(chans));
    aug_check(3 == count_);
    aug_check(1 == last_);

    /* Remove middle. */

    aug_check(AUG_ISSUCCESS(aug_removechan(chans, 2)));

    /* 3 1 */
    foreach_(chans);
    /* 1 3 */
    aug_check(2 == aug_getchans(chans));
    aug_check(2 == count_);
    aug_check(3 == last_);

    /* No longer exists. */

    aug_check(AUG_ISNONE(aug_removechan(chans, 2)));

    /* Remove during loop. */

    cb_ = rm1_;
    aug_processchans(chans);

    /* No longer exists. */

    aug_check(AUG_ISFAIL(aug_removechan(chans, 1)));

    /* 3 */
    foreach_(chans);
    /* 3 */
    aug_check(1 == aug_getchans(chans));
    aug_check(1 == count_);
    aug_check(3 == last_);

    aug_dumpchans(chans);
    aug_destroychans(chans);

     /* Objects released. */

    aug_check(0 == aug_getchans(chans));
    aug_check(0 == refs_);
    aug_release(mpool);
    return 0;
}
