/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/netevent.h"
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
    memset(buf + size, 0, AUG_NEVNAMELEN - size);
}

static void
unpackname_(char* name, const char* buf)
{
    memcpy(name, buf, AUG_NEVNAMELEN);
    name[AUG_NEVNAMELEN] = '\0';
}

AUGNET_API int
aug_verifynetevent(const struct aug_netevent* event)
{
    if ('\0' == event->name_[0]) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null netevent name"));
        return -1;
    }

    if (AUG_NEVHBSEC_MAX < event->hbsec_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum netevent hbsec exceeded"));
        return -1;
    }

    if (AUG_NEVWEIGHT_MAX < event->weight_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum netevent weight size exceeded"));
        return -1;
    }

    if (AUG_NEVTYPE_MAX < event->type_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum netevent type size exceeded"));
        return -1;
    }

    return 0;
}

AUGNET_API char*
aug_packnetevent(char* buf, const struct aug_netevent* event)
{
    aug_encode32(buf + AUG_NEVPROTO_OFFSET, (uint32_t)event->proto_);
    packname_(buf + AUG_NEVNAME_OFFSET, event->name_);
    aug_encode32(buf + AUG_NEVSTATE_OFFSET, (uint32_t)event->state_);
    aug_encode32(buf + AUG_NEVSEQ_OFFSET, (uint32_t)event->seq_);
    buf[AUG_NEVHBSEC_OFFSET] = event->hbsec_;
    buf[AUG_NEVWEIGHT_OFFSET] = event->weight_;
    aug_encode16(buf + AUG_NEVTYPE_OFFSET, (uint16_t)event->type_);
    return buf;
}

AUGNET_API struct aug_netevent*
aug_unpacknetevent(struct aug_netevent* event, const char* buf)
{
    event->proto_ = aug_decode32(buf + AUG_NEVPROTO_OFFSET);
    unpackname_(event->name_, buf + AUG_NEVNAME_OFFSET);
    event->state_ = aug_decode32(buf + AUG_NEVSTATE_OFFSET);
    event->seq_ = aug_decode32(buf + AUG_NEVSEQ_OFFSET);
    event->hbsec_ = buf[AUG_NEVHBSEC_OFFSET];
    event->weight_ = buf[AUG_NEVWEIGHT_OFFSET];
    event->type_ = aug_decode16(buf + AUG_NEVTYPE_OFFSET);
    return event;
}
