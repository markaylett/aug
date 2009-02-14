/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGMAR_BODY_H_
#define AUGMAR_BODY_H_

#include "augmar/seq_.h"

struct aug_info_;

AUG_EXTERNC aug_result
aug_setcontent_(aug_seq_t seq, struct aug_info_* info, const void* data,
                unsigned size);

AUG_EXTERNC aug_result
aug_truncate_(aug_seq_t seq, struct aug_info_* info, unsigned size);

AUG_EXTERNC aug_rsize
aug_write_(aug_seq_t seq, struct aug_info_* info, unsigned offset,
           const void* buf, unsigned len);

AUG_EXTERNC const void*
aug_getcontent_(aug_seq_t seq, const struct aug_info_* info);

AUG_EXTERNC aug_rsize
aug_read_(aug_seq_t seq, const struct aug_info_* info, unsigned offset,
          void* buf, unsigned len);

#endif /* AUGMAR_BODY_H_ */
