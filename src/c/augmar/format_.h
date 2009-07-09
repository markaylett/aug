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
#ifndef AUGMAR_FORMAT_H_
#define AUGMAR_FORMAT_H_

#include "augmar/config.h"
#include "augmar/types.h"

#include "augsys/endian.h"

typedef char aug_magic_t[4];

typedef uint16_t aug_verno_t;
#define aug_encodeverno aug_encode16
#define aug_decodeverno aug_decode16

typedef uint16_t aug_fields_t;
#define aug_encodefields aug_encode16
#define aug_decodefields aug_decode16
#define AUG_MAXFIELDS UINT16_MAX

typedef uint32_t aug_hsize_t;
#define aug_encodehsize aug_encode32
#define aug_decodehsize aug_decode32
#define AUG_MAXHSIZE UINT32_MAX

typedef uint32_t aug_bsize_t;
#define aug_encodebsize aug_encode32
#define aug_decodebsize aug_decode32
#define AUG_MAXBSIZE UINT32_MAX

typedef uint16_t aug_nsize_t;
#define aug_encodensize aug_encode16
#define aug_decodensize aug_decode16
#define AUG_MAXNSIZE UINT16_MAX

typedef uint16_t aug_vsize_t;
#define aug_encodevsize aug_encode16
#define aug_decodevsize aug_decode16
#define AUG_MAXVSIZE UINT16_MAX

#define AUG_LEADER 0
#define AUG_MAGICOFF 0
#define AUG_VERNOOFF (AUG_MAGICOFF + sizeof(aug_magic_t))
#define AUG_FIELDSOFF (AUG_VERNOOFF + sizeof(aug_verno_t))
#define AUG_HSIZEOFF (AUG_FIELDSOFF + sizeof(aug_fields_t))
#define AUG_BSIZEOFF (AUG_HSIZEOFF + sizeof(aug_hsize_t))
#define AUG_LEADERSIZE (AUG_BSIZEOFF + sizeof(aug_bsize_t))

#define AUG_HEADER AUG_LEADERSIZE

#define AUG_NSIZEOFF 0
#define AUG_VSIZEOFF (AUG_NSIZEOFF + sizeof(aug_nsize_t))
#define AUG_NAMEOFF (AUG_VSIZEOFF + sizeof(aug_vsize_t))
#define AUG_VALUEOFF(nsize) (AUG_NAMEOFF + (nsize))
#define AUG_FIELDSIZE(nsize, vsize) (AUG_VALUEOFF(nsize) + (vsize))

#define AUG_BODY(hsize) (AUG_HEADER + hsize)

#define AUG_MAGIC "mar"

#endif /* AUGMAR_FORMAT_H_ */
