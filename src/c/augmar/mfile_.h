/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * @file mfile.h
 *
 * TODO
 */

#ifndef AUGMAR_MFILE_H_
#define AUGMAR_MFILE_H_

#include "augmar/config.h"
#include "augmar/types.h"

typedef struct aug_mfile_* aug_mfile_t;

AUG_EXTERNC int
aug_closemfile_(aug_mfile_t mfile);

AUG_EXTERNC aug_mfile_t
aug_openmfile_(const char* path, int flags, mode_t mode, unsigned tail);

AUG_EXTERNC void*
aug_mapmfile_(aug_mfile_t mfile, unsigned size);

AUG_EXTERNC int
aug_syncmfile_(aug_mfile_t mfile);

AUG_EXTERNC int
aug_truncatemfile_(aug_mfile_t mfile, unsigned size);

AUG_EXTERNC void*
aug_mfileaddr_(aug_mfile_t mfile);

AUG_EXTERNC unsigned
aug_mfileresvd_(aug_mfile_t mfile);

AUG_EXTERNC unsigned
aug_mfilesize_(aug_mfile_t mfile);

AUG_EXTERNC void*
aug_mfiletail_(aug_mfile_t mfile);

#endif /* AUGMAR_MFILE_H_ */
