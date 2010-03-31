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
#ifndef AUGMAR_MFILE_H_
#define AUGMAR_MFILE_H_

#include "augmar/config.h"
#include "augmar/types.h"

#include "augext/mpool.h"

#include "augtypes.h"

typedef struct aug_mfile_* aug_mfile_t;

AUG_EXTERNC aug_result
aug_closemfile_AI_(aug_mfile_t mfile);

AUG_EXTERNC aug_mfile_t
aug_openmfile_IN_(aug_mpool* mpool, const char* path, int flags, mode_t mode,
                  unsigned tail);

AUG_EXTERNC void*
aug_mapmfile_AIN_(aug_mfile_t mfile, unsigned size);

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
