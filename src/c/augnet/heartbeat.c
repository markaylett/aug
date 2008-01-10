/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/heartbeat.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/endian.h"
#include "augmar/format_.h"

#include <string.h>

#define HEAD_OFFSET_ 0
#define NAME_OFFSET_ (HEAD_OFFSET_ + sizeof(uint32_t))
#define SEQ_OFFSET_ (NAME_OFFSET_ + AUG_HBNAMELEN)
#define STATE_OFFSET_ (SEQ_OFFSET_ + sizeof(uint32_t))
#define LOAD_OFFSET_ (STATE_OFFSET_ + sizeof(uint32_t))
#define HBINT_OFFSET_ (LOAD_OFFSET_ + sizeof(char))
#define MSG_OFFSET_ (HBINT_OFFSET_ + sizeof(char))

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

AUGNET_API char*
aug_packheartbeat(char* buf, const struct aug_heartbeat* hb)
{
    aug_encode32(buf + HEAD_OFFSET_, (uint32_t)hb->head_);
    packname_(buf + NAME_OFFSET_, hb->name_);
    aug_encode32(buf + SEQ_OFFSET_, (uint32_t)hb->seq_);
    aug_encode32(buf + STATE_OFFSET_, (uint32_t)hb->state_);
    buf[LOAD_OFFSET_] = hb->load_;
    buf[HBINT_OFFSET_] = hb->hbint_;
    aug_encode16(buf + MSG_OFFSET_, (uint16_t)hb->msg_);
    return buf;
}

AUGNET_API struct aug_heartbeat*
aug_unpackheartbeat(struct aug_heartbeat* hb, const char* buf)
{
    hb->head_ = aug_decode32(buf + HEAD_OFFSET_);
    unpackname_(hb->name_, buf);
    hb->seq_ = aug_decode32(buf + SEQ_OFFSET_);
    hb->state_ = aug_decode32(buf + STATE_OFFSET_);
    hb->load_ = buf[LOAD_OFFSET_];
    hb->hbint_ = buf[HBINT_OFFSET_];
    hb->msg_ = aug_decode16(buf + MSG_OFFSET_);
    return hb;
}
