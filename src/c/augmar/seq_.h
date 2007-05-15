/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file seq.h
   TODO
 */

#ifndef AUGMAR_SEQ_H_
#define AUGMAR_SEQ_H_

#include "augmar/config.h"
#include "augmar/types.h"

typedef struct aug_seq_* aug_seq_t;

AUG_EXTERN int
aug_destroyseq_(aug_seq_t seq);

AUG_EXTERN int
aug_copyseq_(aug_seq_t dst, aug_seq_t src);

AUG_EXTERN aug_seq_t
aug_createseq_(unsigned tail);

AUG_EXTERN aug_seq_t
aug_openseq_(const char* path, int flags, mode_t mode, unsigned tail);

AUG_EXTERN void*
aug_resizeseq_(aug_seq_t seq, unsigned size);

AUG_EXTERN int
aug_setregion_(aug_seq_t seq, unsigned offset, unsigned len);

AUG_EXTERN int
aug_syncseq_(aug_seq_t seq);

AUG_EXTERN void*
aug_seqaddr_(aug_seq_t seq);

AUG_EXTERN unsigned
aug_seqsize_(aug_seq_t seq);

AUG_EXTERN void*
aug_seqtail_(aug_seq_t seq);

#endif /* AUGMAR_SEQ_H_ */
