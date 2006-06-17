/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file body.h
 * \brief TODO
 */

#ifndef AUGMAR_BODY_H_
#define AUGMAR_BODY_H_

#include "augmar/seq_.h"

struct aug_info_;

AUGMAR_EXTERN int
aug_setcontent_(aug_seq_t seq, struct aug_info_* info, const void* data,
                size_t size);

AUGMAR_EXTERN int
aug_truncate_(aug_seq_t seq, struct aug_info_* info, size_t size);

AUGMAR_EXTERN ssize_t
aug_write_(aug_seq_t seq, struct aug_info_* info, size_t offset,
           const void* buf, size_t size);

AUGMAR_EXTERN const void*
aug_content_(aug_seq_t seq, const struct aug_info_* info);

AUGMAR_EXTERN ssize_t
aug_read_(aug_seq_t seq, const struct aug_info_* info, size_t offset,
          void* buf, size_t size);

#endif /* AUGMAR_BODY_H_ */
