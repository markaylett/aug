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
#ifndef AUGMAR_HEADER_H_
#define AUGMAR_HEADER_H_

#include "augmar/seq_.h"

struct aug_info_;

AUG_EXTERNC aug_rint
aug_clearfields_AIN_(aug_seq_t seq, struct aug_info_* info);

AUG_EXTERNC aug_result
aug_delfieldn_AIN_(aug_seq_t seq, struct aug_info_* info, unsigned n);

AUG_EXTERNC aug_rint
aug_delfieldp_AIN_(aug_seq_t seq, struct aug_info_* info, const char* name);

AUG_EXTERNC aug_rint
aug_getfieldn_(aug_seq_t seq, const struct aug_info_* info, unsigned n,
               const void** value);

AUG_EXTERNC aug_rint
aug_getfieldp_(aug_seq_t seq, const struct aug_info_* info,
               const char* name, const void** value);

AUG_EXTERNC aug_result
aug_getfield_(aug_seq_t seq, const struct aug_info_* info, unsigned n,
              struct aug_field* field);

AUG_EXTERNC aug_result
aug_putfieldn_AIN_(aug_seq_t seq, struct aug_info_* info, unsigned n,
                   const void* value, unsigned size);

AUG_EXTERNC aug_rint
aug_putfieldp_AIN_(aug_seq_t seq, struct aug_info_* info, const char* name,
                   const void* value, unsigned size);

AUG_EXTERNC aug_result
aug_fieldntop_(aug_seq_t seq, const struct aug_info_* info, unsigned n,
               const char** name);

AUG_EXTERNC aug_rint
aug_fieldpton_(aug_seq_t seq, const struct aug_info_* info, const char* name);

#endif /* AUGMAR_HEADER_H_ */
