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
#ifndef AUGMAR_FORMAT_H_
#define AUGMAR_FORMAT_H_

#include "augmar/config.h"
#include "augmar/types.h"

#include "augsys/endian.h"

typedef uint16_t aug_verno_t;
#define aug_encodeverno aug_encode16
#define aug_decodeverno aug_decode16

typedef uint16_t aug_fields_t;
#define aug_encodefields aug_encode16
#define aug_decodefields aug_decode16
#define AUG_FIELDS_MAX UINT16_MAX

typedef uint32_t aug_hsize_t;
#define aug_encodehsize aug_encode32
#define aug_decodehsize aug_decode32
#define AUG_HSIZE_MAX UINT32_MAX

typedef uint32_t aug_bsize_t;
#define aug_encodebsize aug_encode32
#define aug_decodebsize aug_decode32
#define AUG_BSIZE_MAX UINT32_MAX

typedef uint16_t aug_nsize_t;
#define aug_encodensize aug_encode16
#define aug_decodensize aug_decode16
#define AUG_NSIZE_MAX UINT16_MAX

typedef uint16_t aug_vsize_t;
#define aug_encodevsize aug_encode16
#define aug_decodevsize aug_decode16
#define AUG_VSIZE_MAX UINT16_MAX

#define AUG_LEADER 0
#define AUG_VERNO_OFFSET 0
#define AUG_FIELDS_OFFSET (AUG_VERNO_OFFSET + sizeof(aug_verno_t))
#define AUG_HSIZE_OFFSET (AUG_FIELDS_OFFSET + sizeof(aug_fields_t))
#define AUG_BSIZE_OFFSET (AUG_HSIZE_OFFSET + sizeof(aug_hsize_t))
#define AUG_LEADER_SIZE (AUG_BSIZE_OFFSET + sizeof(aug_bsize_t))

#define AUG_HEADER AUG_LEADER_SIZE

#define AUG_NSIZE_OFFSET 0
#define AUG_VSIZE_OFFSET (AUG_NSIZE_OFFSET + sizeof(aug_nsize_t))
#define AUG_NAME_OFFSET (AUG_VSIZE_OFFSET + sizeof(aug_vsize_t))
#define AUG_VALUE_OFFSET(nsize) (AUG_NAME_OFFSET + (nsize))
#define AUG_FIELD_SIZE(nsize, vsize) (AUG_VALUE_OFFSET(nsize) + (vsize))

#define AUG_BODY(hsize) (AUG_HEADER + hsize)

#endif /* AUGMAR_FORMAT_H_ */
