/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/info_.h"

#include "augmar/format_.h"

#include <assert.h>

AUGMAR_EXTERN int
aug_setinfo_(aug_seq_t seq, const struct aug_info_* info)
{
    aug_byte_t* addr;
    assert(seq && info);

    if (-1 == aug_setregion_(seq, 0, AUG_LEADER_SIZE))
        return -1;

    if (!(addr = (aug_byte_t*)aug_seqaddr_(seq)))
        return -1;

    aug_encodeverno_(addr + AUG_VERNO_OFFSET, (aug_verno_t)info->verno_);
    aug_encodefields_(addr + AUG_FIELDS_OFFSET, (aug_fields_t)info->fields_);
    aug_encodehsize_(addr + AUG_HSIZE_OFFSET, (aug_hsize_t)info->hsize_);
    aug_encodebsize_(addr + AUG_BSIZE_OFFSET, (aug_bsize_t)info->bsize_);
    return 0;
}

AUGMAR_EXTERN int
aug_info_(aug_seq_t seq, struct aug_info_* info)
{
    aug_byte_t* addr;
    assert(seq && info);

    if (-1 == aug_setregion_(seq, 0, AUG_LEADER_SIZE))
        return -1;

    if (!(addr = (aug_byte_t*)aug_seqaddr_(seq)))
        return -1;

    info->verno_ = aug_decodeverno_(addr + AUG_VERNO_OFFSET);
    info->fields_ = aug_decodefields_(addr + AUG_FIELDS_OFFSET);
    info->hsize_ = aug_decodehsize_(addr + AUG_HSIZE_OFFSET);
    info->bsize_ = aug_decodebsize_(addr + AUG_BSIZE_OFFSET);
    return 0;
}
