/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
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
aug_copyseq_(aug_seq_t dst, aug_seq_t src);

AUG_EXTERNC aug_seq_t
aug_createseq_(aug_mpool* mpool, unsigned tail);

AUG_EXTERNC aug_seq_t
aug_openseq_(aug_mpool* mpool, const char* path, int flags, mode_t mode,
             unsigned tail);

AUG_EXTERNC void*
aug_resizeseq_(aug_seq_t seq, unsigned size);

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
