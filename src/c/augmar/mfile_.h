/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file mfile.h
 * \brief TODO
 */

#ifndef AUGMAR_MFILE_H_
#define AUGMAR_MFILE_H_

#include "augmar/config.h"
#include "augmar/types.h"

typedef struct aug_mfile_* aug_mfile_t;

AUGMAR_EXTERN int
aug_closemfile_(aug_mfile_t mfile);

AUGMAR_EXTERN aug_mfile_t
aug_openmfile_(const char* path, int flags, mode_t mode, size_t tail);

AUGMAR_EXTERN void*
aug_mapmfile_(aug_mfile_t mfile, size_t size);

AUGMAR_EXTERN int
aug_syncmfile_(aug_mfile_t mfile);

AUGMAR_EXTERN int
aug_truncatemfile_(aug_mfile_t mfile, size_t size);

AUGMAR_EXTERN void*
aug_mfileaddr_(aug_mfile_t mfile);

AUGMAR_EXTERN size_t
aug_mfileresvd_(aug_mfile_t mfile);

AUGMAR_EXTERN size_t
aug_mfilesize_(aug_mfile_t mfile);

AUGMAR_EXTERN void*
aug_mfiletail_(aug_mfile_t mfile);

#endif /* AUGMAR_MFILE_H_ */
