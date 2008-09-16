/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGMAR_MFILE_H_
#define AUGMAR_MFILE_H_

#include "augmar/config.h"
#include "augmar/types.h"

#include "augext/mpool.h"

#include "augtypes.h"

typedef struct aug_mfile_* aug_mfile_t;

AUG_EXTERNC aug_result
aug_closemfile_(aug_mfile_t mfile);

AUG_EXTERNC aug_mfile_t
aug_openmfile_(aug_mpool* mpool, const char* path, int flags, mode_t mode,
               unsigned tail);

AUG_EXTERNC void*
aug_mapmfile_(aug_mfile_t mfile, unsigned size);

AUG_EXTERNC aug_result
aug_syncmfile_(aug_mfile_t mfile);

AUG_EXTERNC aug_result
aug_truncatemfile_(aug_mfile_t mfile, unsigned size);

AUG_EXTERNC void*
aug_mfileaddr_(aug_mfile_t mfile);

AUG_EXTERNC unsigned
aug_mfileresvd_(aug_mfile_t mfile);

AUG_EXTERNC aug_mpool*
aug_mfilempool_(aug_mfile_t mfile);

AUG_EXTERNC unsigned
aug_mfilesize_(aug_mfile_t mfile);

AUG_EXTERNC void*
aug_mfiletail_(aug_mfile_t mfile);

#endif /* AUGMAR_MFILE_H_ */
