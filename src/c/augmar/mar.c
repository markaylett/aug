/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/mar.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/body_.h"
#include "augmar/format_.h"
#include "augmar/header_.h"
#include "augmar/info_.h"
#include "augmar/mfile_.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include "augext/log.h"

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
    unsigned offset_, refs_;
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
    unsigned size = aug_seqsize_(seq);
    aug_result result;

    /* An existing archive will be at least as big as the minimum size. */

    if (AUG_LEADER_SIZE <= size) {

        if ((result = aug_info_(seq, info)) < 0)
            return result;

        /* Verify version number embedded within header. */

        if (VERNO != info->verno_) {

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                           AUG_MSG("invalid version number '%d'"),
                           (int)info->verno_);
            return AUG_FAILERROR;
        }
    } else {

        char* ptr;
        if ((result = aug_setregion_(seq, 0, size)) < 0)
            return result;

        if (!(ptr = aug_resizeseq_(seq, AUG_LEADER_SIZE)))
            return AUG_FAILERROR;

        aug_encodeverno(ptr + AUG_VERNO_OFFSET,
                        (aug_verno_t)(info->verno_ = VERNO));
        aug_encodefields(ptr + AUG_FIELDS_OFFSET,
                         (aug_fields_t)(info->fields_ = 0));
        aug_encodehsize(ptr + AUG_HSIZE_OFFSET,
                        (aug_hsize_t)(info->hsize_ = 0));
        aug_encodebsize(ptr + AUG_BSIZE_OFFSET,
                        (aug_bsize_t)(info->bsize_ = 0));
    }

    return AUG_SUCCESS;
}

AUGMAR_API aug_result
aug_copymar(aug_mar_t dst, aug_mar_t src)
{
    aug_result result;

    if (!WRITABLE_(dst)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid destination archive handle"));
        return AUG_FAILERROR;
    }

    if (!READABLE_(src)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid source archive handle"));
        return AUG_FAILERROR;
    }

    if (0 <= (result = aug_copyseq_(dst->seq_, src->seq_)))
        memcpy(&dst->info_, &src->info_, sizeof(dst->info_));

    return result;
}

AUGMAR_API aug_mar_t
aug_createmar(aug_mpool* mpool)
{
    aug_mar_t mar;
    aug_seq_t seq;
    struct aug_info_ info;

    if (!(seq = aug_createseq_(mpool, sizeof(struct aug_mar_))))
        return NULL;

    if (init_(seq, &info) < 0)
        goto fail;

    mar = (aug_mar_t)aug_seqtail_(seq);
    mar->seq_ = seq;
    memcpy(&mar->info_, &info, sizeof(info));
    mar->flags_ = AUG_RDWR;
    mar->offset_ = 0;
    mar->refs_ = 1U;
    return mar;

 fail:
    aug_destroyseq_(seq);
    return NULL;
}

AUGMAR_API aug_mar_t
aug_openmar(aug_mpool* mpool, const char* path, int flags, ...)
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

    if (!(seq = aug_openseq_(mpool, path, flags & ~AUG_TRUNC, mode,
                             sizeof(struct aug_mar_))))
        return NULL;

    if (init_(seq, &info) < 0)
        goto fail;

    mar = (aug_mar_t)aug_seqtail_(seq);
    mar->seq_ = seq;
    memcpy(&mar->info_, &info, sizeof(info));
    mar->flags_ = flags;
    mar->offset_ = 0;
    mar->refs_ = 1U;

    if (flags & AUG_TRUNC) {

        assert(AUG_TRUNC == (flags & AUG_TRUNC));
        if (aug_truncatemar(mar, 0) < 0)
            goto fail;
    }
    return mar;

 fail:
    aug_destroyseq_(seq);
    return NULL;
}

AUGMAR_API void
aug_releasemar(aug_mar_t mar)
{
    if (0 < --mar->refs_)
        return;

    if (WRITABLE_(mar)) {

        /* TODO: warn about corruption if either of these two operations
           fail. */

        struct aug_info_ local;
        if (aug_info_(mar->seq_, &local) < 0)
            goto done;

        if (0 != memcmp(&local, &mar->info_, sizeof(local))
            && aug_setinfo_(mar->seq_, &mar->info_) < 0)
            goto done;
    }

 done:
    aug_destroyseq_(mar->seq_);
}

AUGMAR_API void
aug_retainmar(aug_mar_t mar)
{
    ++mar->refs_;
}

AUGMAR_API aug_result
aug_compactmar(aug_mar_t mar)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS; /* Not implemented. */
}

AUGMAR_API aug_result
aug_removefields(aug_mar_t mar)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_removefields_(mar->seq_, &mar->info_);
}

AUGMAR_API aug_result
aug_setfield(aug_mar_t mar, const struct aug_field* field, unsigned* ord)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_setfield_(mar->seq_, &mar->info_, field, ord);
}

AUGMAR_API aug_result
aug_setvalue(aug_mar_t mar, unsigned ord, const void* value, unsigned size)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_setvalue_(mar->seq_, &mar->info_, ord, value, size);
}

AUGMAR_API aug_result
aug_unsetbyname(aug_mar_t mar, const char* name, unsigned* ord)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_unsetbyname_(mar->seq_, &mar->info_, name, ord);
}

AUGMAR_API aug_result
aug_unsetbyord(aug_mar_t mar, unsigned ord)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_unsetbyord_(mar->seq_, &mar->info_, ord);
}

AUGMAR_API const void*
aug_valuebyname(aug_mar_t mar, const char* name, unsigned* size)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }
    return aug_valuebyname_(mar->seq_, &mar->info_, name, size);
}

AUGMAR_API const void*
aug_valuebyord(aug_mar_t mar, unsigned ord, unsigned* size)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }
    return aug_valuebyord_(mar->seq_, &mar->info_, ord, size);
}

AUGMAR_API aug_result
aug_getfield(aug_mar_t mar, struct aug_field* field, unsigned ord)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_getfield_(mar->seq_, &mar->info_, field, ord);
}

AUGMAR_API unsigned
aug_getfields(aug_mar_t mar)
{
    return mar->info_.fields_;
}

AUGMAR_API aug_result
aug_ordtoname(aug_mar_t mar, const char** name, unsigned ord)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_ordtoname_(mar->seq_, &mar->info_, name, ord);
}

AUGMAR_API aug_result
aug_nametoord(aug_mar_t mar, unsigned* ord, const char* name)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_nametoord_(mar->seq_, &mar->info_, ord, name);
}

AUGMAR_API aug_result
aug_insertmar(aug_mar_t mar, const char* path)
{
    aug_mpool* mpool;
    aug_mfile_t mfile;
    const void* addr;
    unsigned size;

    mpool = aug_seqmpool_(mar->seq_);
    mfile = aug_openmfile_(mpool, path, AUG_RDONLY, 0, 0);
    aug_release(mpool);

    if (!mfile)
        return AUG_FAILERROR;

    if (0 != (size = aug_mfileresvd_(mfile))) {

        aug_result result;

        if (!(addr = aug_mapmfile_(mfile, size))) {
            aug_closemfile_(mfile);
            return AUG_FAILERROR;
        }

        if ((result = aug_setcontent(mar, addr, size)) < 0) {
            aug_closemfile_(mfile);
            return result;
        }
    }

    return aug_closemfile_(mfile);
}

AUGMAR_API off_t
aug_seekmar(aug_mar_t mar, off_t offset, int whence)
{
    off_t local;

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
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid whence value '%d'"), (int)whence);
        return AUG_FAILERROR;
    }

    if (local < 0) {

        /* Assertion? */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ERANGE,
                       AUG_MSG("negative file position '%d'"), (int)local);
        return AUG_FAILERROR;
    }

    mar->offset_ = local;
    return AUG_SUCCESS;
}

AUGMAR_API aug_result
aug_setcontent(aug_mar_t mar, const void* cdata, unsigned size)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_setcontent_(mar->seq_, &mar->info_, cdata, size);
}

AUGMAR_API aug_result
aug_syncmar(aug_mar_t mar)
{
    return aug_syncseq_(mar->seq_);
}

AUGMAR_API aug_result
aug_truncatemar(aug_mar_t mar, unsigned size)
{
    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    /* In keeping with the semantics of ftruncate, this function does not
       modify the offset - the equivalent of the file offset. */

    return aug_truncate_(mar->seq_, &mar->info_, size);
}

AUGMAR_API int
aug_writemar(aug_mar_t mar, const void* buf, unsigned len)
{
    int result;

    if (!WRITABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    if (mar->flags_ & AUG_APPEND) {

        assert(AUG_APPEND == (mar->flags_ & AUG_APPEND));

        if ((result = aug_seekmar(mar, 0, AUG_END)) < 0)
            return result;
    }

    if (0 <= (result = aug_write_(mar->seq_, &mar->info_, mar->offset_, buf,
                                  len)))
        mar->offset_ += result;

    return result;
}

AUGMAR_API aug_result
aug_extractmar(aug_mar_t mar, const char* path)
{
    aug_mpool* mpool;
    aug_mfile_t mfile;
    void* dst;
    const void* src;
    unsigned size;

    if (!(src = aug_getcontent(mar, &size)))
        return AUG_FAILERROR;

    mpool = aug_seqmpool_(mar->seq_);
    mfile = aug_openmfile_(mpool, path, AUG_WRONLY | AUG_CREAT | AUG_TRUNC,
                           0666, 0);
    aug_release(mpool);

    if (!mfile)
        return AUG_FAILERROR;

    if (size) {

        if (!(dst = aug_mapmfile_(mfile, size)))
            goto fail;

        memcpy(dst, src, size);
    }

    return aug_closemfile_(mfile);

 fail:
    aug_closemfile_(mfile);
    return AUG_FAILERROR;
}

AUGMAR_API const void*
aug_getcontent(aug_mar_t mar, unsigned* size)
{
    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }

    *size = mar->info_.bsize_;
    return aug_getcontent_(mar->seq_, &mar->info_);
}

AUGMAR_API int
aug_readmar(aug_mar_t mar, void* buf, unsigned len)
{
    int result;

    if (!READABLE_(mar)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    if (0 <= (result = aug_read_(mar->seq_, &mar->info_, mar->offset_, buf,
                                 len)))
        mar->offset_ += result;

    return result;
}

AUGMAR_API unsigned
aug_getcontentsize(aug_mar_t mar)
{
    return mar->info_.bsize_;
}
