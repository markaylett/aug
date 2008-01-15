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
packstring_(char* dst, const char* src, size_t size)
{
    size_t len = strlen(src);
    memcpy(dst, src, len);
    memset(dst + len, 0, size - len);
}

static void
unpackstring_(char* dst, const char* src, size_t size)
{
    memcpy(dst, src, size);
    dst[size] = '\0';
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

    if (AUG_NEVLOAD_MAX < event->load_) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_ELIMIT,
                       AUG_MSG("maximum netevent load size exceeded"));
        return -1;
    }

    return 0;
}

AUGNET_API char*
aug_packnetevent(char* buf, const struct aug_netevent* event)
{
    aug_encode32(buf + AUG_NEVPROTO_OFFSET, (uint32_t)event->proto_);
    packstring_(buf + AUG_NEVNAME_OFFSET, event->name_, AUG_NEVNAMELEN);
    packstring_(buf + AUG_NEVADDR_OFFSET, event->addr_, AUG_NEVADDRLEN);
    aug_encode32(buf + AUG_NEVTYPE_OFFSET, (uint32_t)event->type_);
    aug_encode32(buf + AUG_NEVSTATE_OFFSET, (uint32_t)event->state_);
    aug_encode32(buf + AUG_NEVSEQ_OFFSET, (uint32_t)event->seq_);
    buf[AUG_NEVHBSEC_OFFSET] = event->hbsec_;
    buf[AUG_NEVWEIGHT_OFFSET] = event->weight_;
    buf[AUG_NEVLOAD_OFFSET] = event->load_;
    buf[AUG_NEVRESV_OFFSET] = 0;
    return buf;
}

AUGNET_API struct aug_netevent*
aug_unpacknetevent(struct aug_netevent* event, const char* buf)
{
    event->proto_ = aug_decode32(buf + AUG_NEVPROTO_OFFSET);
    unpackstring_(event->name_, buf + AUG_NEVNAME_OFFSET, AUG_NEVNAMELEN);
    unpackstring_(event->addr_, buf + AUG_NEVADDR_OFFSET, AUG_NEVADDRLEN);
    event->type_ = aug_decode32(buf + AUG_NEVTYPE_OFFSET);
    event->state_ = aug_decode32(buf + AUG_NEVSTATE_OFFSET);
    event->seq_ = aug_decode32(buf + AUG_NEVSEQ_OFFSET);
    event->hbsec_ = buf[AUG_NEVHBSEC_OFFSET];
    event->weight_ = buf[AUG_NEVWEIGHT_OFFSET];
    event->load_ = buf[AUG_NEVLOAD_OFFSET];
#if 0
    event->resv_ = buf[AUG_NEVRESV_OFFSET];
#endif
    return event;
}
