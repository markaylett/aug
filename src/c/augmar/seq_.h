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
#ifndef AUGMAR_SEQ_H_
#define AUGMAR_SEQ_H_

#include "augmar/config.h"
#include "augmar/types.h"

#include "augext/mpool.h"

#include "augtypes.h"

typedef struct aug_seq_* aug_seq_t;

AUG_EXTERNC void
aug_destroyseq_(aug_seq_t seq);

AUG_EXTERNC aug_result
aug_copyseq_(aug_seq_t src, aug_seq_t dst);

AUG_EXTERNC aug_seq_t
aug_createseq_(aug_mpool* mpool, unsigned tail);

AUG_EXTERNC aug_seq_t
aug_openseq_IN_(aug_mpool* mpool, const char* path, int flags, mode_t mode,
                unsigned tail);

AUG_EXTERNC void*
aug_resizeseq_BIN_(aug_seq_t seq, unsigned size);

AUG_EXTERNC aug_result
aug_setregion_(aug_seq_t seq, unsigned offset, unsigned len);

AUG_EXTERNC aug_result
aug_syncseq_(aug_seq_t seq);

AUG_EXTERNC void*
aug_seqaddr_(aug_seq_t seq);

AUG_EXTERNC aug_mpool*
aug_seqmpool_(aug_seq_t seq);

AUG_EXTERNC unsigned
aug_seqsize_(aug_seq_t seq);

AUG_EXTERNC void*
aug_seqtail_(aug_seq_t seq);

#endif /* AUGMAR_SEQ_H_ */
