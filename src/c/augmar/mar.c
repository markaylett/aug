/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/mar.h"

static const char rcsid[] = "$Id:$";

#include "augmar/body_.h"
#include "augmar/format_.h"
#include "augmar/header_.h"
#include "augmar/info_.h"
#include "augmar/mfile_.h"

#include "augsys/defs.h"
#include "augsys/errinfo.h"
#include "augsys/log.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define READ_  0x0001
#define WRITE_ 0x0002

#define READABLE_(x) ((x) && (torw_((x)->flags_) & READ_))
#define WRITABLE_(x) ((x) && (torw_((x)->flags_) & WRITE_))

struct aug_mar_ {
    aug_seq_t seq_;
    struct aug_info_ info_;
    int flags_;
    size_t offset_, refs_;
};

static unsigned
torw_(int from)
{
    switch (from & (AUG_RDONLY | AUG_WRONLY | AUG_RDWR)) {
    case AUG_RDONLY:
        return READ_;
    case AUG_WRONLY:
        return WRITE_;
    case AUG_RDWR:
        return READ_ | WRITE_;
    }
    return 0;
}

static int
init_(aug_seq_t seq, struct aug_info_* info)
{
    static const aug_verno_t VERNO = 2U;
    size_t size = aug_seqsize_(seq);

    /* An existing archive will be at least as big as the minimum size. */

    if (AUG_LEADER_SIZE <= size) {

        if (-1 == aug_info_(seq, info))
            return -1;

        /* Verify version number embedded within header. */

        if (VERNO != info->verno_) {

            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                           AUG_MSG("invalid version number '%d'"),
                           (int)info->verno_);
            return -1;
        }
    } else {

        char* ptr;
        if (-1 == aug_setregion_(seq, 0, size))
            return -1;

        if (!(ptr = aug_resizeseq_(seq, AUG_LEADER_SIZE)))
            return -1;

        aug_encodeverno(ptr + AUG_VERNO_OFFSET,
                        (aug_verno_t)(info->verno_ = VERNO));
        aug_encodefields(ptr + AUG_FIELDS_OFFSET,
                         (aug_fields_t)(info->fields_ = 0));
        aug_encodehsize(ptr + AUG_HSIZE_OFFSET,
                        (aug_hsize_t)(info->hsize_ = 0));
        aug_encodebsize(ptr + AUG_BSIZE_OFFSET,
                        (aug_bsize_t)(info->bsize_ = 0));
    }

    return 0;
}

AUGMAR_API int
aug_copymar(aug_mar_t dst, aug_mar_t src)
{
    if (!WRITABLE_(dst)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid destination archive handle"));
        return -1;
    }

    if (!READABLE_(src)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid source archive handle"));
        return -1;
    }

    if (-1 == aug_copyseq_(dst->seq_, src->seq_))
        return -1;

    memcpy(&dst->info_, &src->info_, sizeof(dst->info_));
    return 0;
}

AUGMAR_API aug_mar_t
aug_createmar(void)
{
    aug_mar_t mar;
    aug_seq_t seq;
    struct aug_info_ info;

    if (!(seq = aug_createseq_(sizeof(struct aug_mar_))))
        return NULL;

    if (-1 == init_(seq, &info))
        goto fail;

    mar = (aug_mar_t)aug_seqtail_(seq);
    mar->seq_ = seq;
    memcpy(&mar->info_, &info, sizeof(info));
    mar->flags_ = AUG_RDWR;
    mar->offset_ = 0;
    mar->refs_ = 1U;
    return mar;

 fail:
    aug_freeseq_(seq);
    return NULL;
}

AUGMAR_API aug_mar_t
aug_openmar(const char* path, int flags, ...)
{
    aug_mar_t mar;
    aug_seq_t seq;
    struct aug_info_ info;
    mode_t mode;

    if (flags & AUG_CREAT) {

        va_list args;
        assert(AUG_CREAT == (flags & AUG_CREAT));
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);

    } else
        mode = 0;

    if (!(seq = aug_openseq_(path, flags & ~AUG_TRUNC, mode,
                             sizeof(struct aug_mar_))))
        return NULL;

    if (-1 == init_(seq, &info))
        goto fail;

    mar = (aug_mar_t)aug_seqtail_(seq);
    mar->seq_ = seq;
    memcpy(&mar->info_, &info, sizeof(info));
    mar->flags_ = flags;
    mar->offset_ = 0;
    mar->refs_ = 1U;

    if (flags & AUG_TRUNC) {

        assert(AUG_TRUNC == (flags & AUG_TRUNC));
        if (-1 == aug_truncatemar(mar, 0))
            goto fail;
    }
    return mar;

 fail:
    aug_freeseq_(seq);
    return NULL;
}

AUGMAR_API int
aug_releasemar(aug_mar_t mar)
{
    if (!mar) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null archive handle"));
        return -1;
    }

    if (0 < --mar->refs_)
        return 0;

    if (WRITABLE_(mar)) {

        struct aug_info_ local;
        if (-1 == aug_info_(mar->seq_, &local))
            goto fail;

        if (0 != memcmp(&local, &mar->info_, sizeof(local))
            && -1 == aug_setinfo_(mar->seq_, &mar->info_))
            goto fail;
    }

    return aug_freeseq_(mar->seq_);

 fail:
    aug_freeseq_(mar->seq_);
    return -1;
}

AUGMAR_API int
aug_retainmar(aug_mar_t mar)
{
    if (!mar) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null archive handle"));
        return -1;
    }
    ++mar->refs_;
    return 0;
}

AUGMAR_API int
aug_compactmar(aug_mar_t mar)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return 0; /* Not implemented. */
}

AUGMAR_API int
aug_removefields(aug_mar_t mar)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_removefields_(mar->seq_, &mar->info_);
}

AUGMAR_API int
aug_setfield(aug_mar_t mar, const struct aug_field* field, size_t* ord)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_setfield_(mar->seq_, &mar->info_, field, ord);
}

AUGMAR_API int
aug_setvalue(aug_mar_t mar, size_t ord, const void* value, size_t size)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_setvalue_(mar->seq_, &mar->info_, ord, value, size);
}

AUGMAR_API int
aug_unsetbyname(aug_mar_t mar, const char* name, size_t* ord)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_unsetbyname_(mar->seq_, &mar->info_, name, ord);
}

AUGMAR_API int
aug_unsetbyord(aug_mar_t mar, size_t ord)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_unsetbyord_(mar->seq_, &mar->info_, ord);
}

AUGMAR_API const void*
aug_valuebyname(aug_mar_t mar, const char* name, size_t* size)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }
    return aug_valuebyname_(mar->seq_, &mar->info_, name, size);
}

AUGMAR_API const void*
aug_valuebyord(aug_mar_t mar, size_t ord, size_t* size)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }
    return aug_valuebyord_(mar->seq_, &mar->info_, ord, size);
}

AUGMAR_API int
aug_field(aug_mar_t mar, struct aug_field* field, size_t ord)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_field_(mar->seq_, &mar->info_, field, ord);
}

AUGMAR_API int
aug_fields(aug_mar_t mar, size_t* size)
{
    if (!mar) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null archive handle"));
        return -1;
    }
    if (!size) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null size pointer"));
        return -1;
    }
    *size = mar->info_.fields_;
    return 0;
}

AUGMAR_API int
aug_ordtoname(aug_mar_t mar, const char** name, size_t ord)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_ordtoname_(mar->seq_, &mar->info_, name, ord);
}

AUGMAR_API int
aug_nametoord(aug_mar_t mar, size_t* ord, const char* name)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_nametoord_(mar->seq_, &mar->info_, ord, name);
}

AUGMAR_API int
aug_insertmar(aug_mar_t mar, const char* path)
{
    aug_mfile_t mfile;
    const void* addr;
    size_t size;

    if (!(mfile = aug_openmfile_(path, AUG_RDONLY, 0, 0)))
        return -1;

    if (0 != (size = aug_mfileresvd_(mfile))) {

        if (!(addr = aug_mapmfile_(mfile, size)))
            goto fail;

        if (-1 == aug_setcontent(mar, addr, size))
            goto fail;
    }
    return aug_closemfile_(mfile);

 fail:
    aug_closemfile_(mfile);
    return -1;
}

AUGMAR_API off_t
aug_seekmar(aug_mar_t mar, off_t offset, int whence)
{
    off_t local;

    if (!mar) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null archive handle"));
        return -1;
    }

    switch (whence) {
    case AUG_SET:
        local = offset;
        break;
    case AUG_CUR:
        local = (off_t)(mar->offset_ + offset);
        break;
    case AUG_END:
        local = (off_t)(mar->info_.bsize_ + offset);
        break;
    default:
        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid whence value '%d'"), (int)whence);
        return -1;
    }
    if (local < 0) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                       AUG_MSG("negative file position '%d'"), (int)local);
        return -1;
    }

    mar->offset_ = local;
    return 0;
}

AUGMAR_API int
aug_setcontent(aug_mar_t mar, const void* cdata, size_t size)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }
    return aug_setcontent_(mar->seq_, &mar->info_, cdata, size);
}

AUGMAR_API int
aug_syncmar(aug_mar_t mar)
{
    if (!mar) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null archive handle"));
        return -1;
    }
    return aug_syncseq_(mar->seq_);
}

AUGMAR_API int
aug_truncatemar(aug_mar_t mar, size_t size)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }

    /* In keeping with the semantics of ftruncate, this function does not
       modify the offset - the equivalent of the file offset. */

    return aug_truncate_(mar->seq_, &mar->info_, size);
}

AUGMAR_API ssize_t
aug_writemar(aug_mar_t mar, const void* buf, size_t size)
{
    ssize_t ret;

    if (!WRITABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }

    if (mar->flags_ & AUG_APPEND) {
        assert(AUG_APPEND == (mar->flags_ & AUG_APPEND));
        if (-1 == aug_seekmar(mar, 0, AUG_END))
            return -1;
    }

    if (-1 == (ret = aug_write_(mar->seq_, &mar->info_, mar->offset_, buf,
                                size)))
        return -1;

    mar->offset_ += ret;
    return ret;
}

AUGMAR_API int
aug_extractmar(aug_mar_t mar, const char* path)
{
    aug_mfile_t mfile;
    void* dst;
    const void* src;
    size_t size;

    if (!(src = aug_content(mar, &size)))
        return -1;

    if (!(mfile = aug_openmfile_(path, AUG_WRONLY | AUG_CREAT | AUG_TRUNC,
                                 0666, 0)))
        return -1;

    if (size) {

        if (!(dst = aug_mapmfile_(mfile, size)))
            goto fail;

        memcpy(dst, src, size);
    }

    return aug_closemfile_(mfile);

 fail:
    aug_closemfile_(mfile);
    return -1;
}

AUGMAR_API const void*
aug_content(aug_mar_t mar, size_t* size)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }

    *size = mar->info_.bsize_;
    return aug_content_(mar->seq_, &mar->info_);
}

AUGMAR_API ssize_t
aug_readmar(aug_mar_t mar, void* buf, size_t size)
{
    ssize_t ret;

    if (!READABLE_(mar)) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return -1;
    }

    if (-1 == (ret = aug_read_(mar->seq_, &mar->info_, mar->offset_, buf,
                               size)))
        return -1;

    mar->offset_ += ret;
    return ret;
}

AUGMAR_API int
aug_contentsize(aug_mar_t mar, size_t* size)
{
    if (!mar) {

        aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_ENULL,
                       AUG_MSG("null archive handle"));
        return -1;
    }
    *size = mar->info_.bsize_;
    return 0;
}
