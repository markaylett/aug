/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/heartbeat.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/endian.h"
#include "augsys/errinfo.h"

#include <string.h>

static void
packname_(char* buf, const char* name)
{
    size_t size = strlen(name);
    memcpy(buf, name, size);
    memset(buf + size, 0, AUG_HBNAMELEN - size);
}

static void
unpackname_(char* name, const char* buf)
{
    memcpy(name, buf, AUG_HBNAMELEN);
    name[AUG_HBNAMELEN] = '\0';
}

AUGNET_API int
aug_verifyheartbeat(const struct aug_heartbeat* hb)
{
    if ('\0' == hb->name_[0]) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null heartbeat name"));
        return -1;
    }

    if (AUG_HBLOAD_MAX < hb->load_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum heartbeat load size exceeded"));
        return -1;
    }

    if (AUG_HBSEC_MAX < hb->sec_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum heartbeat seconds size exceeded"));
        return -1;
    }

    if (AUG_HBMSG_MAX < hb->msg_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum heartbeat message size exceeded"));
        return -1;
    }

    return 0;
}

AUGNET_API char*
aug_packheartbeat(char* buf, const struct aug_heartbeat* hb)
{
    aug_encode32(buf + AUG_HBHEAD_OFFSET, (uint32_t)hb->head_);
    packname_(buf + AUG_HBNAME_OFFSET, hb->name_);
    aug_encode32(buf + AUG_HBSEQ_OFFSET, (uint32_t)hb->seq_);
    aug_encode32(buf + AUG_HBSTATE_OFFSET, (uint32_t)hb->state_);
    buf[AUG_HBLOAD_OFFSET] = hb->load_;
    buf[AUG_HBSEC_OFFSET] = hb->sec_;
    aug_encode16(buf + AUG_HBMSG_OFFSET, (uint16_t)hb->msg_);
    return buf;
}

AUGNET_API struct aug_heartbeat*
aug_unpackheartbeat(struct aug_heartbeat* hb, const char* buf)
{
    hb->head_ = aug_decode32(buf + AUG_HBHEAD_OFFSET);
    unpackname_(hb->name_, buf);
    hb->seq_ = aug_decode32(buf + AUG_HBSEQ_OFFSET);
    hb->state_ = aug_decode32(buf + AUG_HBSTATE_OFFSET);
    hb->load_ = buf[AUG_HBLOAD_OFFSET];
    hb->sec_ = buf[AUG_HBSEC_OFFSET];
    hb->msg_ = aug_decode16(buf + AUG_HBMSG_OFFSET);
    return hb;
}
