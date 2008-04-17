/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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
offsetbyord_(void* begin, unsigned ord)
{
    char* ptr = begin;
    while (ord--)
        ptr += fieldsize_(ptr);

    return (unsigned)(ptr - (char*)begin);
}

static unsigned
offsetbyname_(void* begin, const char* name, unsigned* inout)
{
    /* The inout parameter contains the number of fields on input, and the
       matching ordinal on output. */

    char* ptr = begin;
    unsigned ord;

    for (ord = 0; ord < *inout; ++ord) {

        unsigned nsize = nsize_(ptr);
        if (0 == aug_strncasecmp(name, name_(ptr), nsize)) {
            *inout = ord;
            break;
        }

        ptr += fieldsize_(ptr);
    }

    return (unsigned)(ptr - (char*)begin);
}

AUG_EXTERNC int
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

AUG_EXTERNC int
aug_setfield_(aug_seq_t seq, struct aug_info_* info,
              const struct aug_field* field, unsigned* ord)
{
    char* ptr;
    unsigned nsize, vsize, fsize, inout, offset, orig;
    assert(seq && info && field);

    if (!field->name_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ENULL,
                       AUG_MSG("null field name"));
        return -1;
    }

    nsize = (unsigned)strlen(field->name_) + 1;
    vsize = field->size_ + 1; /* Add null terminator. */

    if (AUG_NSIZE_MAX < nsize) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum field-name size exceeded"));
        return -1;
    }

    if (AUG_VSIZE_MAX < vsize) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum field-value size exceeded"));
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

    info->hsize_ += (int)fsize - (int)orig;

    /* Set optional output parameter. */

    if (ord)
        *ord = inout;

    return 0;
}

AUG_EXTERNC int
aug_setvalue_(aug_seq_t seq, struct aug_info_* info, unsigned ord,
              const void* value, unsigned size)
{
    char* ptr;
    unsigned offset, nsize, vsize, orig;
    assert(seq && info);

    if (ord >= info->fields_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                       AUG_MSG("field '%d' does not exist"), (int)ord);
        return -1;
    }

    /* Add null terminator. */

    vsize = size + 1;

    if (AUG_VSIZE_MAX < vsize) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ELIMIT,
                       AUG_MSG("maximum field-value size exceeded"));
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

    info->hsize_ += (int)vsize - (int)orig;
    return 0;
}

AUG_EXTERNC int
aug_unsetbyname_(aug_seq_t seq, struct aug_info_* info, const char* name,
                 unsigned* ord)
{
    char* ptr;
    unsigned inout, offset, orig;
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
        return AUG_RETNOMATCH;
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

AUG_EXTERNC int
aug_unsetbyord_(aug_seq_t seq, struct aug_info_* info, unsigned ord)
{
    char* ptr;
    unsigned offset, orig;

    if (ord >= info->fields_)
        return AUG_RETNOMATCH;

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

AUG_EXTERNC const void*
aug_valuebyname_(aug_seq_t seq, const struct aug_info_* info,
                 const char* name, unsigned* size)
{
    char* ptr;
    unsigned inout;
    assert(seq && info && name);

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return NULL;

    if (!(ptr = aug_seqaddr_(seq)))
        return NULL;

    /* Move pointer to required field. */

    inout = info->fields_;
    ptr += offsetbyname_(ptr, name, &inout);

    if (inout == info->fields_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                       AUG_MSG("field '%s' does not exist"), name);
        return NULL;
    }

    /* Set optional output parameter: minus null terminator. */

    if (size)
        *size = vsize_(ptr) - 1;

    return value_(ptr, nsize_(ptr));
}

AUG_EXTERNC const void*
aug_valuebyord_(aug_seq_t seq, const struct aug_info_* info, unsigned ord,
                unsigned* size)
{
    char* ptr;
    assert(seq && info);

    if (ord >= info->fields_) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EEXIST,
                       AUG_MSG("field '%d' does not exist"), (int)ord);
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

AUG_EXTERNC int
aug_getfield_(aug_seq_t seq, const struct aug_info_* info,
              struct aug_field* field, unsigned ord)
{
    char* ptr;

    if (ord >= info->fields_) {

        /* Return empty field on detection of out-of-bounds condition. */

        field->name_ = NULL;
        field->value_ = NULL;
        field->size_ = 0;
        return AUG_RETNOMATCH;
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

AUG_EXTERNC int
aug_ordtoname_(aug_seq_t seq, const struct aug_info_* info, const char** name,
               unsigned ord)
{
    char* ptr;
    assert(seq && info && name);

    if (ord >= info->fields_) {
        *name = NULL;
        return AUG_RETNOMATCH;
    }

    if (-1 == aug_setregion_(seq, AUG_HEADER, info->hsize_))
        return -1;

    if (!(ptr = aug_seqaddr_(seq)))
        return -1;

    *name = name_(ptr + offsetbyord_(ptr, ord));
    return 0;
}

AUG_EXTERNC int
aug_nametoord_(aug_seq_t seq, const struct aug_info_* info, unsigned* ord,
               const char* name)
{
    char* ptr;
    unsigned inout;
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

    return inout == info->fields_ ? AUG_RETNOMATCH : 0;
}
