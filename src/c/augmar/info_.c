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
