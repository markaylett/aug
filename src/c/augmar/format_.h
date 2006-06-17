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

#if !defined(_MSC_VER)

# include <inttypes.h>

#else /* _MSC_VER */

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

#endif /* _MSC_VER */

typedef uint8_t aug_byte_t;
typedef uint16_t aug_uint16_t;
typedef uint32_t aug_uint32_t;

#define AUG_UINT16_MAX 0xffffU
#define AUG_UINT32_MAX 0xffffffffU

typedef aug_uint16_t aug_verno_t;
#define aug_encodeverno_ aug_encode16_
#define aug_decodeverno_ aug_decode16_

typedef aug_uint16_t aug_fields_t;
#define aug_encodefields_ aug_encode16_
#define aug_decodefields_ aug_decode16_
#define AUG_FIELDS_MAX AUG_UINT16_MAX

typedef aug_uint32_t aug_hsize_t;
#define aug_encodehsize_ aug_encode32_
#define aug_decodehsize_ aug_decode32_
#define AUG_HSIZE_MAX AUG_UINT32_MAX

typedef aug_uint32_t aug_bsize_t;
#define aug_encodebsize_ aug_encode32_
#define aug_decodebsize_ aug_decode32_
#define AUG_BSIZE_MAX AUG_UINT32_MAX

typedef aug_uint16_t aug_nsize_t;
#define aug_encodensize_ aug_encode16_
#define aug_decodensize_ aug_decode16_
#define AUG_NSIZE_MAX AUG_UINT16_MAX

typedef aug_uint16_t aug_vsize_t;
#define aug_encodevsize_ aug_encode16_
#define aug_decodevsize_ aug_decode16_
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

AUGMAR_EXTERN aug_uint16_t
aug_decode16_(const aug_byte_t* ptr);

AUGMAR_EXTERN aug_uint32_t
aug_decode32_(const aug_byte_t* ptr);

AUGMAR_EXTERN void
aug_encode16_(aug_byte_t* ptr, aug_uint16_t i);

AUGMAR_EXTERN void
aug_encode32_(aug_byte_t* ptr, aug_uint32_t i);

#endif /* AUGMAR_FORMAT_H_ */
