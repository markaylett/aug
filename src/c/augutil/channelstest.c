/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augctx.h"

#include <stdio.h>

static int refs_ = 0;

static void*
cast_(aug_channelob* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_channelobid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retain_(aug_channelob* ob)
{
    ++refs_;
}

static void
release_(aug_channelob* ob)
{
    --refs_;
}

static aug_result
close_(aug_channelob* ob)
{
    return AUG_SUCCESS;
}

static aug_bool
dispatch_(aug_channelob* ob, aug_channelcb_t cb)
{
    return cb(ob);
}

static aug_result
setnonblock_(aug_channelob* ob, aug_bool on)
{
    return AUG_SUCCESS;
}

static aug_result
seteventmask_(aug_channelob* ob, unsigned short mask)
{
    return AUG_SUCCESS;
}

static int
eventmask_(aug_channelob* ob)
{
    return 0;
}

static int
events_(aug_channelob* ob)
{
    return 0;
}

static const struct aug_channelobvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    close_,
    dispatch_,
    setnonblock_,
    seteventmask_,
    eventmask_,
    events_
};

static aug_channelob channel1_ = { &vtbl_, NULL };
static aug_channelob channel2_ = { &vtbl_, NULL };
static aug_channelob channel3_ = { &vtbl_, NULL };

static int count_ = 0;
static int last_ = 0;

static aug_bool
cb_(aug_channelob* ob)
{
    ++count_;
    if (ob == &channel1_) {
        last_ = 1;
    } else if (ob == &channel2_) {
        last_ = 2;
    } else if (ob == &channel3_) {
        last_ = 3;
    } else {
        aug_die("invalid channel");
    }
    return AUG_TRUE;
}

static void
foreach_(aug_channels_t channels)
{
    count_ = 0;
    last_ = 0;
    aug_foreachchannel(channels, cb_);
}

static aug_bool
rm1_(aug_channelob* ob)
{
    return ob == &channel1_ ? AUG_FALSE : AUG_TRUE;
}

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    aug_channels_t channels;

    aug_check(0 <= aug_start());

    mpool = aug_getmpool(aug_tlx);
    aug_check(mpool);

    channels = aug_createchannels(mpool);
    aug_check(channels);

    aug_check(0 <= aug_insertchannel(channels, &channel1_));
    aug_check(0 <= aug_insertchannel(channels, &channel2_));
    aug_check(0 <= aug_insertchannel(channels, &channel3_));
    aug_check(3 == refs_);

    foreach_(channels);
    aug_check(3 == aug_getchannels(channels));
    aug_check(3 == count_);
    aug_check(3 == last_);

    /* Fairness rotation. */

    foreach_(channels);
    aug_check(3 == aug_getchannels(channels));
    aug_check(3 == count_);
    aug_check(1 == last_);

    /* Fairness rotation. */

    foreach_(channels);
    aug_check(3 == aug_getchannels(channels));
    aug_check(3 == count_);
    aug_check(2 == last_);

    /* Remove middle. */

    aug_check(0 <= aug_removechannel(channels, &channel2_));

    foreach_(channels);
    aug_check(2 == aug_getchannels(channels));
    aug_check(2 == count_);
    aug_check(3 == last_);

    /* No longer exists. */

    aug_check(AUG_FAILNONE == aug_removechannel(channels, &channel2_));

    /* Remove during loop. */

    aug_foreachchannel(channels, rm1_);

    /* No longer exists. */

    aug_check(AUG_FAILNONE == aug_removechannel(channels, &channel1_));

    foreach_(channels);
    aug_check(1 == aug_getchannels(channels));
    aug_check(1 == count_);
    aug_check(3 == last_);

    aug_destroychannels(channels);

     /* Objects released. */

    aug_check(0 == aug_getchannels(channels));
    aug_check(0 == refs_);
    aug_release(mpool);
    return 0;
}
