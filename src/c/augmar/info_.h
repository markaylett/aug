/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGMAR_INFO_H_
#define AUGMAR_INFO_H_

#include "augmar/seq_.h"

struct aug_info_ {
    unsigned verno_, fields_, hsize_, bsize_;
};

AUG_EXTERNC aug_result
aug_setinfo_(aug_seq_t seq, const struct aug_info_* info);

AUG_EXTERNC aug_result
aug_info_(aug_seq_t seq, struct aug_info_* info);

#endif /* AUGMAR_INFO_H_ */
