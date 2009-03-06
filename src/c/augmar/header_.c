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
#define AUGMAR_BUILD
#include "augmar/header_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/format_.h"
#include "augmar/info_.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/string.h"

#include <assert.h>
#include <string.h>

static void
setnsize_(const void* field, aug_nsize_t nsize)
{
    aug_encodensize((char*)field + AUG_NSIZE_OFFSET, nsize);
}

static unsigned
nsize_(const void* field)
{
    return aug_decodensize((char*)field + AUG_NSIZE_OFFSET);
}

static void
setvsize_(const void* field, aug_vsize_t vsize)
{
    aug_encodevsize((char*)field + AUG_VSIZE_OFFSET, vsize);
}

static unsigned
vsize_(const void* field)
{
    return aug_decodevsize((char*)field + AUG_VSIZE_OFFSET);
}

static const char*
name_(void* field)
{
    return (const char*)field + AUG_NAME_OFFSET;
}

static void*
value_(void* field, unsigned nsize)
{
    return (char*)field + AUG_VALUE_OFFSET(nsize);
}

static unsigned
fieldsize_(const void* field)
{
    unsigned nsize = nsize_(field);
    unsigned vsize = vsize_(field);
    return AUG_FIELD_SIZE(nsize, vsize);
}

static unsigned
offsetn_(void* begin, unsigned n)
{
    char* ptr = begin;
    while (n--)
        ptr += fieldsize_(ptr);

    return (unsigned)(ptr - (char*)begin);
}

static unsigned
offsetp_(void* begin, const char* name, unsigned* inout)
{
    /* The inout parameter contains the number of fields on input, and the
       matching ordinal on output. */

    char* ptr = begin;
    unsigned n;

    for (n = 0; n < *inout; ++n) {

        unsigned nsize = nsize_(ptr);
        if (0 == aug_strncasecmp(name, name_(ptr), nsize)) {
            *inout = n;
            break;
        }

        ptr += fieldsize_(ptr);
    }

    return (unsigned)(ptr - (char*)begin);
}

AUG_EXTERNC aug_rint
aug_clearfields_(aug_seq_t seq, struct aug_info_* info)
{
    unsigned orig;

    assert(seq && info);

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!aug_resizeseq_(seq, 0))
        return AUG_FAILERROR;

    orig = info->fields_;
    info->fields_ = 0;
    info->hsize_ = 0;
    return AUG_MKRESULT(orig);
}

AUG_EXTERNC aug_result
aug_delfieldn_(aug_seq_t seq, struct aug_info_* info, unsigned n)
{
    unsigned offset, orig;
    char* ptr;

    if (n >= info->fields_)
        return AUG_FAILNONE;

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    offset = offsetn_(ptr, n);
    orig = fieldsize_(ptr + offset);

    aug_verify(aug_setregion_(seq, AUG_HEADER + offset, orig));

    if (!(ptr = aug_resizeseq_(seq, 0)))
        return AUG_FAILERROR;

    --info->fields_;
    info->hsize_ -= orig;

    return AUG_SUCCESS;
}

AUG_EXTERNC aug_rint
aug_delfieldp_(aug_seq_t seq, struct aug_info_* info, const char* name)
{
    unsigned inout, offset, orig;
    char* ptr;
    assert(seq && info && name);

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    inout = info->fields_;
    offset = offsetp_(ptr, name, &inout);

    if (inout == info->fields_)
        return AUG_FAILNONE;

    orig = fieldsize_(ptr + offset);
    aug_verify(aug_setregion_(seq, AUG_HEADER + offset, orig));

    if (!(ptr = aug_resizeseq_(seq, 0)))
        return AUG_FAILERROR;

    --info->fields_;
    info->hsize_ -= orig;

    return AUG_MKRESULT(inout);
}

AUG_EXTERNC aug_rint
aug_getfieldn_(aug_seq_t seq, const struct aug_info_* info, unsigned n,
               const void** value)
{
    char* ptr;

    assert(seq && info);

    if (n >= info->fields_) {
        *value = NULL;
        return AUG_MKRESULT(0);
    }

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    /* Move pointer to required field. */

    ptr += offsetn_(ptr, n);

    /* Set output parameter. */

    *value = value_(ptr, nsize_(ptr));

    /* Minus null terminator. */

    return AUG_MKRESULT(vsize_(ptr) - 1);
}

AUG_EXTERNC aug_rint
aug_getfieldp_(aug_seq_t seq, const struct aug_info_* info,
               const char* name, const void** value)
{
    unsigned inout;
    char* ptr;

    assert(seq && info && name);

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    /* Move pointer to required field. */

    inout = info->fields_;
    ptr += offsetp_(ptr, name, &inout);

    if (inout == info->fields_) {
        *value = NULL;
        return AUG_MKRESULT(0);
    }

    /* Set output parameter. */

    *value = value_(ptr, nsize_(ptr));

    /* Minus null terminator. */

    return AUG_MKRESULT(vsize_(ptr) - 1);
}

AUG_EXTERNC aug_result
aug_getfield_(aug_seq_t seq, const struct aug_info_* info,
              unsigned n, struct aug_field* field)
{
    char* ptr;

    if (n >= info->fields_) {

        /* Return empty field on detection of out-of-bounds condition. */

        field->name_ = NULL;
        field->value_ = NULL;
        field->size_ = 0;

        return AUG_MKRESULT(0);
    }

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    /* Move pointer to required field. */

    ptr += offsetn_(ptr, n);

    field->name_ = name_(ptr);
    field->value_ = value_(ptr, nsize_(ptr));
    field->size_ = vsize_(ptr) - 1; /* Minus null terminator. */
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_result
aug_putfieldn_(aug_seq_t seq, struct aug_info_* info, unsigned n,
               const void* value, unsigned size)
{
    unsigned offset, nsize, vsize, orig;
    char* ptr;

    assert(seq && info);

    if (n >= info->fields_)
        return AUG_FAILNONE;

    /* Add null terminator. */

    vsize = size + 1;

    if (AUG_VSIZE_MAX < vsize) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum field-value size exceeded"));
        return AUG_FAILERROR;
    }

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    offset = offsetn_(ptr, n);
    ptr += offset;

    nsize = nsize_(ptr);
    orig = vsize_(ptr);

    aug_verify(aug_setregion_(seq, AUG_HEADER + offset,
                              AUG_FIELD_SIZE(nsize, orig)));

    if (!(ptr = aug_resizeseq_(seq, AUG_FIELD_SIZE(nsize, vsize))))
        return AUG_FAILERROR;

    /* Set field value. */

    setvsize_(ptr, (aug_vsize_t)vsize);
    if (size)
        memcpy(ptr + AUG_VALUE_OFFSET(nsize), value, size);

    /* Always null terminate. */

    ptr[AUG_VALUE_OFFSET(nsize) + size] = '\0';

    /* Add difference between old and new value size to header size. */

    info->hsize_ += (int)vsize - (int)orig;
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_rint
aug_putfieldp_(aug_seq_t seq, struct aug_info_* info, const char* name,
               const void* value, unsigned size)
{
    unsigned nsize, vsize, fsize, inout, offset, orig;
    char* ptr;

    assert(seq && info);

    if (!name) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ENULL,
                       AUG_MSG("null field name"));
        return AUG_FAILERROR;
    }

    nsize = (unsigned)strlen(name) + 1;
    vsize = size + 1; /* Add null terminator. */

    if (AUG_NSIZE_MAX < nsize) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum field-name size exceeded"));
        return AUG_FAILERROR;
    }

    if (AUG_VSIZE_MAX < vsize) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum field-value size exceeded"));
        return AUG_FAILERROR;
    }

    fsize = AUG_FIELD_SIZE(nsize, vsize);

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    inout = info->fields_;
    offset = offsetp_(ptr, name, &inout);

    /* A new field has an original size of zero. */

    orig = inout == info->fields_ ? 0 : fieldsize_(ptr + offset);

    aug_verify(aug_setregion_(seq, AUG_HEADER + offset, orig));

    if (!(ptr = aug_resizeseq_(seq, fsize)))
        return AUG_FAILERROR;

    /* Set field name. */

    setnsize_(ptr, (aug_nsize_t)nsize);
    memcpy(ptr + AUG_NAME_OFFSET, name, nsize);

    /* Set field value */

    setvsize_(ptr, (aug_vsize_t)vsize);
    if (size)
        memcpy(ptr + AUG_VALUE_OFFSET(nsize), value, size);

    /* Always null terminate. */

    ptr[AUG_VALUE_OFFSET(nsize) + size] = '\0';

    /* If new field then increment field count. */

    if (inout == info->fields_)
        ++info->fields_;

    /* Add difference between old and new field size to header size. */

    info->hsize_ += (int)fsize - (int)orig;

    return AUG_MKRESULT(inout);
}

AUG_EXTERNC aug_result
aug_fieldntop_(aug_seq_t seq, const struct aug_info_* info, unsigned n,
               const char** name)
{
    char* ptr;

    assert(seq && info && name);

    if (n >= info->fields_) {
        *name = NULL;
        return AUG_FAILNONE;
    }

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    *name = name_(ptr + offsetn_(ptr, n));
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_rint
aug_fieldpton_(aug_seq_t seq, const struct aug_info_* info, const char* name)
{
    char* ptr;
    unsigned inout;

    assert(seq && info && name);

    aug_verify(aug_setregion_(seq, AUG_HEADER, info->hsize_));

    if (!(ptr = aug_seqaddr_(seq)))
        return AUG_FAILERROR;

    inout = info->fields_;
    offsetp_(ptr, name, &inout);

    if (inout == info->fields_)
        return AUG_FAILNONE;

    return AUG_MKRESULT(inout);
}
