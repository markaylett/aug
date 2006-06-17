/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file seq.h
 * \brief TODO
 */

#ifndef AUGMAR_SEQ_H
#define AUGMAR_SEQ_H

#include "augmar/config.h"
#include "augmar/types.h"

typedef struct aug_seq_* aug_seq_t;

AUGMAR_EXTERN int
aug_freeseq_(aug_seq_t seq);

AUGMAR_EXTERN int
aug_copyseq_(aug_seq_t dst, aug_seq_t src);

AUGMAR_EXTERN aug_seq_t
aug_createseq_(size_t tail);

AUGMAR_EXTERN aug_seq_t
aug_openseq_(const char* path, int flags, mode_t mode, size_t tail);

AUGMAR_EXTERN void*
aug_resizeseq_(aug_seq_t seq, size_t size);

AUGMAR_EXTERN int
aug_setregion_(aug_seq_t seq, size_t offset, size_t len);

AUGMAR_EXTERN int
aug_syncseq_(aug_seq_t seq);

AUGMAR_EXTERN void*
aug_seqaddr_(aug_seq_t seq);

AUGMAR_EXTERN size_t
aug_seqsize_(aug_seq_t seq);

AUGMAR_EXTERN void*
aug_seqtail_(aug_seq_t seq);

#endif /* AUGMAR_SEQ_H */
