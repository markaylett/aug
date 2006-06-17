/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/header_.h"

#include "augmar/format_.h"
#include "augmar/info_.h"

#include "augsys/log.h"
#include "augsys/string.h"

#include <assert.h>
#include <errno.h>
#include <string.h>

static void
setnsize_(const void* field, aug_nsize_t nsize)
{
    aug_encodensize_((aug_byte_t*)field + AUG_NSIZE_OFFSET, nsize);
}

static size_t
nsize_(const void* field)
{
    return aug_decodensize_((aug_byte_t*)field + AUG_NSIZE_OFFSET);
}

static void
setvsize_(const void* field, aug_vsize_t vsize)
{
    aug_encodevsize_((aug_byte_t*)field + AUG_VSIZE_OFFSET, vsize);
}

static size_t
vsize_(const void* field)
{
    return aug_decodevsize_((aug_byte_t*)field + AUG_VSIZE_OFFSET);
}

static const char*
name_(void* field)
{
    return (const char*)((aug_byte_t*)field + AUG_NAME_OFFSET);
}

static void*
value_(void* field, size_t nsize)
{
    return (aug_byte_t*)field + AUG_VALUE_OFFSET(nsize);
}

static size_t
fieldsize_(const void* field)
{
    size_t nsize = nsize_(field);
    size_t vsize = vsize_(field);
    return AUG_FIELD_SIZE(nsize, vsize);
}

static size_t
offsetbyord_(void* begin, size_t ord)
{
    aug_byte_t* ptr = begin;
    while (ord--)
        ptr += fieldsize_(ptr);

    return ptr - (aug_byte_t*)begin;
}

static size_t
offsetbyname_(void* begin, const char* name, size_t* inout)
{
    /* The inout parameter contains the number of fields on input, and the
       matching ordinal on output. */

    aug_byte_t* ptr = begin;
    size_t ord;

    for (ord = 0; ord < *inout; ++ord) {

        size_t nsize = nsize_(ptr);
        if (0 == aug_strncasecmp(name, name_(ptr), nsize)) {
            *inout = ord;
            break;
        }

        ptr += fieldsize_(ptr);
    }

    return ptr - (aug_byte_t*)begin;
}

AUGMAR_EXTERN int
aug_removefields_(aug_seq_t seq, struct aug_info_* info)
{
    assert(seq && info);

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!aug_resizeseq_(seq, 0))
        return -1;

    info->fields_ = 0;
    info->hsize_ = 0;
    return 0;
}

AUGMAR_EXTERN int
aug_setfield_(aug_seq_t seq, struct aug_info_* info,
              const struct aug_field* field, size_t* ord)
{
    aug_byte_t* ptr;
    size_t nsize, vsize, fsize, inout, offset, orig;
    assert(seq && info && field);

    if (!field->name_) {

        errno = EINVAL;
        aug_error("null field name");
        return -1;
    }

    nsize = strlen(field->name_) + 1;
    vsize = field->size_ + 1; /* Add null terminator. */

    if (AUG_NSIZE_MAX < nsize) {

        errno = EINVAL;
        aug_error("maximum field name size exceeded");
        return -1;
    }

    if (AUG_VSIZE_MAX < vsize) {

        errno = EINVAL;
        aug_error("maximum field value size exceeded");
        return -1;
    }

    fsize = AUG_FIELD_SIZE(nsize, vsize);

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    inout = info->fields_;
    offset = offsetbyname_(ptr, field->name_, &inout);

    /* A new field has an original size of zero. */

    orig = inout == info->fields_ ? 0 : fieldsize_(ptr + offset);

    if (-1 == aug_setregion_(seq, AUG_HEADER + offset, orig))
        return -1;

    if (!(ptr = aug_resizeseq_(seq, fsize)))
        return -1;

    /* Set field name. */

    setnsize_(ptr, (aug_nsize_t)nsize);
    memcpy(ptr + AUG_NAME_OFFSET, field->name_, nsize);

    /* Set field value */

    setvsize_(ptr, (aug_vsize_t)vsize);
    if (field->size_)
        memcpy(ptr + AUG_VALUE_OFFSET(nsize), field->value_, field->size_);

    /* Always null terminate. */

    ptr[AUG_VALUE_OFFSET(nsize) + field->size_] = '\0';

    /* If new field then increment field count. */

    if (inout == info->fields_)
        ++info->fields_;

    /* Add difference between old and new field size to header size. */

    info->hsize_ += (ssize_t)fsize - (ssize_t)orig;

    /* Set optional output parameter. */

    if (ord)
        *ord = inout;

    return 0;
}

AUGMAR_EXTERN int
aug_setvalue_(aug_seq_t seq, struct aug_info_* info, size_t ord,
              const void* value, size_t size)
{
    aug_byte_t* ptr;
    size_t offset, nsize, vsize, orig;
    assert(seq && info);

    if (ord >= info->fields_) {

        errno = EINVAL;
        aug_error("field '%d' does not exist", ord);
        return -1;
    }

    /* Add null terminator. */

    vsize = size + 1;

    if (AUG_VSIZE_MAX < vsize) {

        errno = EINVAL;
        aug_error("maximum field value size exceeded");
        return -1;
    }

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    offset = offsetbyord_(ptr, ord);
    ptr += offset;

    nsize = nsize_(ptr);
    orig = vsize_(ptr);

    if (-1 == aug_setregion_(seq, AUG_HEADER + offset,
                             AUG_FIELD_SIZE(nsize, orig)))
        return -1;

    if (!(ptr = aug_resizeseq_(seq, AUG_FIELD_SIZE(nsize, vsize))))
        return -1;

    /* Set field value. */

    setvsize_(ptr, (aug_vsize_t)vsize);
    if (size)
        memcpy(ptr + AUG_VALUE_OFFSET(nsize), value, size);

    /* Always null terminate. */

    ptr[AUG_VALUE_OFFSET(nsize) + size] = '\0';

    /* Add difference between old and new value size to header size. */

    info->hsize_ += (ssize_t)vsize - (ssize_t)orig;
    return 0;
}

AUGMAR_EXTERN int
aug_unsetbyname_(aug_seq_t seq, struct aug_info_* info, const char* name,
                 size_t* ord)
{
    aug_byte_t* ptr;
    size_t inout, offset, orig;
    assert(seq && info && name);

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    inout = info->fields_;
    offset = offsetbyname_(ptr, name, &inout);

    if (inout == info->fields_) {

        if (ord)
            *ord = inout;
        return AUG_NOMATCH;
    }

    orig = fieldsize_(ptr + offset);
    if (-1 == aug_setregion_(seq, AUG_HEADER + offset, orig))
        return -1;

    if (!(ptr = aug_resizeseq_(seq, 0)))
        return -1;

    --info->fields_;
    info->hsize_ -= orig;

    /* Set optional output parameter. */

    if (ord)
        *ord = inout;

    return 0;
}

AUGMAR_EXTERN int
aug_unsetbyord_(aug_seq_t seq, struct aug_info_* info, size_t ord)
{
    aug_byte_t* ptr;
    size_t offset, orig;

    if (ord >= info->fields_)
        return AUG_NOMATCH;

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    offset = offsetbyord_(ptr, ord);
    orig = fieldsize_(ptr + offset);

    if (-1 == aug_setregion_(seq, AUG_HEADER + offset, orig))
        return -1;

    if (!(ptr = aug_resizeseq_(seq, 0)))
        return -1;

    --info->fields_;
    info->hsize_ -= orig;
    return 0;
}

AUGMAR_EXTERN const void*
aug_valuebyname_(aug_seq_t seq, const struct aug_info_* info,
                 const char* name, size_t* size)
{
    aug_byte_t* ptr;
    size_t inout;
    assert(seq && info && name);

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return NULL;

    if (!(ptr = aug_seqaddr_(seq)))
        return NULL;

    /* Move pointer to required field. */

    inout = info->fields_;
    ptr += offsetbyname_(ptr, name, &inout);

    if (inout == info->fields_) {

        errno = EINVAL;
        aug_error("field '%s' does not exist", name);
        return NULL;
    }

    /* Set optional output parameter: minus null terminator. */

    if (size)
        *size = vsize_(ptr) - 1;

    return value_(ptr, nsize_(ptr));
}

AUGMAR_EXTERN const void*
aug_valuebyord_(aug_seq_t seq, const struct aug_info_* info, size_t ord,
                size_t* size)
{
    aug_byte_t* ptr;
    assert(seq && info);

    if (ord >= info->fields_) {

        errno = EINVAL;
        aug_error("field '%d' does not exist", ord);
        return NULL;
    }

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return NULL;

    if (!(ptr = aug_seqaddr_(seq)))
        return NULL;

    /* Move pointer to required field. */

    ptr += offsetbyord_(ptr, ord);

    /* Set optional output parameter: minus null terminator. */

    if (size)
        *size = vsize_(ptr) - 1;

    return value_(ptr, nsize_(ptr));
}

AUGMAR_EXTERN int
aug_field_(aug_seq_t seq, const struct aug_info_* info,
           struct aug_field* field, size_t ord)
{
    aug_byte_t* ptr;

    if (ord >= info->fields_) {

        /* Return empty field on detection of out-of-bounds condition. */

        field->name_ = NULL;
        field->value_ = NULL;
        field->size_ = 0;
        return AUG_NOMATCH;
    }

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    /* Move pointer to required field. */

    ptr += offsetbyord_(ptr, ord);

    field->name_ = name_(ptr);
    field->value_ = value_(ptr, nsize_(ptr));
    field->size_ = vsize_(ptr) - 1; /* Minus null terminator. */
    return 0;
}

AUGMAR_EXTERN int
aug_ordtoname_(aug_seq_t seq, const struct aug_info_* info, const char** name,
               size_t ord)
{
    aug_byte_t* ptr;
    assert(seq && info && name);

    if (ord >= info->fields_) {
        *name = NULL;
        return AUG_NOMATCH;
    }

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    *name = name_(ptr + offsetbyord_(ptr, ord));
    return 0;
}

AUGMAR_EXTERN int
aug_nametoord_(aug_seq_t seq, const struct aug_info_* info, size_t* ord,
               const char* name)
{
    aug_byte_t* ptr;
    size_t inout;
    assert(seq && info && name);

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    inout = info->fields_;
    offsetbyname_(ptr, name, &inout);

    /* Set optional output parameter. */

    if (ord)
        *ord = inout;

    return inout == info->fields_ ? AUG_NOMATCH : 0;
}
