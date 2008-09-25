/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/info_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/format_.h"

#include <assert.h>

AUG_EXTERNC aug_result
aug_setinfo_(aug_seq_t seq, const struct aug_info_* info)
{
    char* addr;

    assert(seq && info);

    aug_verify(aug_setregion_(seq, 0, AUG_LEADER_SIZE));

    if (!(addr = (char*)aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    aug_encodeverno(addr + AUG_VERNO_OFFSET, (aug_verno_t)info->verno_);
    aug_encodefields(addr + AUG_FIELDS_OFFSET, (aug_fields_t)info->fields_);
    aug_encodehsize(addr + AUG_HSIZE_OFFSET, (aug_hsize_t)info->hsize_);
    aug_encodebsize(addr + AUG_BSIZE_OFFSET, (aug_bsize_t)info->bsize_);

    return AUG_SUCCESS;
}

AUG_EXTERNC aug_result
aug_info_(aug_seq_t seq, struct aug_info_* info)
{
    char* addr;

    assert(seq && info);

    aug_verify(aug_setregion_(seq, 0, AUG_LEADER_SIZE));

    if (!(addr = (char*)aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    info->verno_ = aug_decodeverno(addr + AUG_VERNO_OFFSET);
    info->fields_ = aug_decodefields(addr + AUG_FIELDS_OFFSET);
    info->hsize_ = aug_decodehsize(addr + AUG_HSIZE_OFFSET);
    info->bsize_ = aug_decodebsize(addr + AUG_BSIZE_OFFSET);

    return AUG_SUCCESS;
}
