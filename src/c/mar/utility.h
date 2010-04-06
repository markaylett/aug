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
#ifndef MAR_UTILITY_H
#define MAR_UTILITY_H

#include "mar/config.h"

#include "augmar/mar.h"

#include <stdio.h>

AUG_EXTERNC aug_result
aug_atofield_(struct aug_field* field, char* src);

AUG_EXTERNC aug_bool
aug_confirm_(const char* prompt);

AUG_EXTERNC aug_result
aug_insertstream_BIN_(aug_mar* mar, FILE* stream);

AUG_EXTERNC aug_rsize
aug_readline_(char* buf, size_t size, FILE* stream);

AUG_EXTERNC aug_result
aug_streamset_BIN_(aug_mar* mar, FILE* stream);

AUG_EXTERNC aug_result
aug_writevalue_(FILE* stream, const void* value, size_t size);

#endif /* MAR_UTILITY_H */
