/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file format.h
 * \brief TODO
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
#define AUG_FIELDS_MAX AUG_UINT16_MAX

typedef uint32_t aug_hsize_t;
#define aug_encodehsize aug_encode32
#define aug_decodehsize aug_decode32
#define AUG_HSIZE_MAX AUG_UINT32_MAX

typedef uint32_t aug_bsize_t;
#define aug_encodebsize aug_encode32
#define aug_decodebsize aug_decode32
#define AUG_BSIZE_MAX AUG_UINT32_MAX

typedef uint16_t aug_nsize_t;
#define aug_encodensize aug_encode16
#define aug_decodensize aug_decode16
#define AUG_NSIZE_MAX AUG_UINT16_MAX

typedef uint16_t aug_vsize_t;
#define aug_encodevsize aug_encode16
#define aug_decodevsize aug_decode16
#define AUG_VSIZE_MAX AUG_UINT16_MAX

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
