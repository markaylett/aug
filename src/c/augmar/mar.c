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

#include "augext/blob.h"
#include "augext/log.h"

#include <assert.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

#define READ_  0x0001
#define WRITE_ 0x0002

#define READABLE_(x) ((x) && (torw_((x)->flags_) & READ_))
#define WRITABLE_(x) ((x) && (torw_((x)->flags_) & WRITE_))

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

static aug_result
init_(aug_seq_t seq, struct aug_info_* info)
{
    static const aug_verno_t VERNO = 3U;
    unsigned size = aug_seqsize_(seq);

    /* An existing archive will be at least as big as the minimum size. */

    if (AUG_LEADERSIZE <= size) {

        aug_verify(aug_info_(seq, info));

        /* Verify version number embedded within header. */

        if (VERNO != info->verno_) {

            aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                           AUG_MSG("invalid version number [%d]"),
                           (int)info->verno_);
            return AUG_FAILERROR;
        }
    } else {

        char* ptr;

        aug_verify(aug_setregion_(seq, 0, size));

        if (!(ptr = aug_resizeseq_(seq, AUG_LEADERSIZE)))
            return AUG_FAILERROR;

        memcpy(ptr + AUG_MAGICOFF, AUG_MAGIC, sizeof(aug_magic_t));
        aug_encodeverno(ptr + AUG_VERNOOFF,
                        (aug_verno_t)(info->verno_ = VERNO));
        aug_encodefields(ptr + AUG_FIELDSOFF,
                         (aug_fields_t)(info->fields_ = 0));
        aug_encodehsize(ptr + AUG_HSIZEOFF,
                        (aug_hsize_t)(info->hsize_ = 0));
        aug_encodebsize(ptr + AUG_BSIZEOFF,
                        (aug_bsize_t)(info->bsize_ = 0));
    }

    return AUG_SUCCESS;
}

struct impl_ {
    aug_mar mar_;
    aug_blob blob_;
    int refs_;
    aug_mpool* mpool_;
    aug_seq_t seq_;
    struct aug_info_ info_;
    int flags_;
    unsigned offset_;
};

static void
destroy_(struct impl_* impl)
{
    if (WRITABLE_(impl)) {

        /* TODO: warn about corruption if either of these two operations
           fail. */

        struct aug_info_ local;
        if (AUG_ISFAIL(aug_info_(impl->seq_, &local)))
            goto done;

        if (0 != memcmp(&local, &impl->info_, sizeof(local))
            && AUG_ISFAIL(aug_setinfo_(impl->seq_, &impl->info_)))
            goto done;
    }

 done:
    aug_destroyseq_(impl->seq_);
}

static void*
cast_(struct impl_* impl, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, aug_marid)) {
        aug_retain(&impl->mar_);
        return &impl->mar_;
    } else if (AUG_EQUALID(id, aug_blobid)) {
        aug_retain(&impl->blob_);
        return &impl->blob_;
    }
    return NULL;
}

static void
retain_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    ++impl->refs_;
}

static void
release_(struct impl_* impl)
{
    assert(0 < impl->refs_);
    if (0 == --impl->refs_) {
        aug_mpool* mpool = impl->mpool_;
        destroy_(impl);
        aug_release(mpool);
    }
}

static void*
mar_cast_(aug_mar* obj, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    return cast_(impl, id);
}

static void
mar_retain_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    retain_(impl);
}

static void
mar_release_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    release_(impl);
}

static aug_result
compact_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return AUG_SUCCESS; /* Not implemented. */
}

static aug_rint
clear_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_clearfields_(impl->seq_, &impl->info_);
}

static aug_result
deln_(aug_mar* obj, unsigned n)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_delfieldn_(impl->seq_, &impl->info_, n);
}

static aug_rint
delp_(aug_mar* obj, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_delfieldp_(impl->seq_, &impl->info_, name);
}

static aug_rint
getn_(aug_mar* obj, unsigned n, const void** value)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_getfieldn_(impl->seq_, &impl->info_, n, value);
}

static aug_rint
getp_(aug_mar* obj, const char* name, const void** value)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_getfieldp_(impl->seq_, &impl->info_, name, value);
}

static aug_result
get_(aug_mar* obj, unsigned n, struct aug_field* field)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_getfield_(impl->seq_, &impl->info_, n, field);
}

static aug_result
putn_(aug_mar* obj, unsigned n, const void* value, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_putfieldn_(impl->seq_, &impl->info_, n, value, size);
}

static aug_rint
putp_(aug_mar* obj, const char* name, const void* value, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_putfieldp_(impl->seq_, &impl->info_, name, value, size);
}

static aug_rint
put_(aug_mar* obj, const struct aug_field* field)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_putfieldp_(impl->seq_, &impl->info_, field->name_,
                          field->value_, field->size_);
}

static aug_result
ntop_(aug_mar* obj, unsigned n, const char** name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_fieldntop_(impl->seq_, &impl->info_, n, name);
}

static aug_rint
pton_(aug_mar* obj, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_fieldpton_(impl->seq_, &impl->info_, name);
}

static unsigned
getcount_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    return impl->info_.fields_;
}

static aug_result
insert_(aug_mar* obj, const char* path)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_mpool* mpool;
    aug_mfile_t mfile;
    const void* addr;
    unsigned size;

    mpool = aug_seqmpool_(impl->seq_);
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

        if (AUG_ISFAIL(result = aug_setcontent(obj, addr, size))) {
            aug_closemfile_(mfile);
            return result;
        }
    }

    return aug_closemfile_(mfile);
}

static aug_rsize
seek_(aug_mar* obj, off_t offset, int whence)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    off_t local;

    switch (whence) {
    case AUG_SET:
        local = offset;
        break;
    case AUG_CUR:
        local = (off_t)(impl->offset_ + offset);
        break;
    case AUG_END:
        local = (off_t)(impl->info_.bsize_ + offset);
        break;
    default:
        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid whence value [%d]"), (int)whence);
        return AUG_FAILERROR;
    }

    if (local < 0) {

        /* Assertion? */

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ERANGE,
                       AUG_MSG("negative file position [%d]"), (int)local);
        return AUG_FAILERROR;
    }

    impl->offset_ = local;
    return AUG_MKRESULT(local);
}

static aug_result
setcontent_(aug_mar* obj, const void* cdata, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }
    return aug_setcontent_(impl->seq_, &impl->info_, cdata, size);
}

static aug_result
sync_(aug_mar* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    return aug_syncseq_(impl->seq_);
}

static aug_result
truncate_(aug_mar* obj, unsigned size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    /* In keeping with the semantics of ftruncate, this function does not
       modify the offset - the equivalent of the file offset. */

    return aug_truncate_(impl->seq_, &impl->info_, size);
}

static aug_rsize
write_(aug_mar* obj, const void* buf, unsigned len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_rsize rsize;

    if (!WRITABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    if (impl->flags_ & AUG_APPEND) {

        assert(AUG_APPEND == (impl->flags_ & AUG_APPEND));

        if (AUG_ISFAIL(rsize = aug_seekmar(obj, 0, AUG_END)))
            return rsize;
    }

    if (AUG_ISSUCCESS(rsize = aug_write_(impl->seq_, &impl->info_,
                                         impl->offset_, buf, len)))
        impl->offset_ += AUG_RESULT(rsize);

    return rsize;
}

static aug_result
extract_(aug_mar* obj, const char* path)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_mpool* mpool;
    aug_mfile_t mfile;
    void* dst;
    const void* src;
    unsigned size;

    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    size = impl->info_.bsize_;
    if (!(src = aug_getcontent_(impl->seq_, &impl->info_)))
        return AUG_FAILERROR;

    mpool = aug_seqmpool_(impl->seq_);
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

static aug_rsize
read_(aug_mar* obj, void* buf, unsigned len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, mar_, obj);
    aug_rsize rsize;

    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return AUG_FAILERROR;
    }

    if (AUG_ISSUCCESS(rsize = aug_read_(impl->seq_, &impl->info_,
                                        impl->offset_, buf, len)))
        impl->offset_ += AUG_RESULT(rsize);

    return rsize;
}

static const struct aug_marvtbl marvtbl_ = {
    mar_cast_,
    mar_retain_,
    mar_release_,
    compact_,
    clear_,
    deln_,
    delp_,
    getn_,
    getp_,
    get_,
    putn_,
    putp_,
    put_,
    ntop_,
    pton_,
    getcount_,
    insert_,
    seek_,
    setcontent_,
    sync_,
    truncate_,
    write_,
    extract_,
    read_,
};

static void*
blob_cast_(aug_blob* obj, const char* id)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    return cast_(impl, id);
}

static void
blob_retain_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    retain_(impl);
}

static void
blob_release_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    release_(impl);
}

static const char*
gettype_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    const void* value;
    if (!AUG_ISSUCCESS(aug_getfieldp_(impl->seq_, &impl->info_,
                                      "Content-Type", &value)))
        value = "application/octet-stream";
    return value;
}

static const void*
getdata_(aug_blob* obj, size_t* size)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    if (!READABLE_(impl)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EINVAL,
                       AUG_MSG("invalid archive handle"));
        return NULL;
    }

    *size = impl->info_.bsize_;
    return aug_getcontent_(impl->seq_, &impl->info_);
}

static size_t
getsize_(aug_blob* obj)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, blob_, obj);
    return impl->info_.bsize_;
}

static const struct aug_blobvtbl blobvtbl_ = {
    blob_cast_,
    blob_retain_,
    blob_release_,
    gettype_,
    getdata_,
    getsize_
};

AUGMAR_API aug_mar*
aug_createmar(aug_mpool* mpool)
{
    aug_seq_t seq;
    struct aug_info_ info;
    struct impl_* impl;

    if (!(seq = aug_createseq_(mpool, sizeof(struct impl_))))
        return NULL;

    if (AUG_ISFAIL(init_(seq, &info))) {
        aug_destroyseq_(seq);
        return NULL;
    }

    impl = (struct impl_*)aug_seqtail_(seq);

    impl->mar_.vtbl_ = &marvtbl_;
    impl->mar_.impl_ = NULL;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->seq_ = seq;
    memcpy(&impl->info_, &info, sizeof(info));
    impl->flags_ = AUG_RDWR;
    impl->offset_ = 0;

    aug_retain(mpool);
    return &impl->mar_;
}

AUGMAR_API aug_mar*
aug_openmar(aug_mpool* mpool, const char* path, int flags, ...)
{
    aug_seq_t seq;
    struct aug_info_ info;
    mode_t mode;
    struct impl_* impl;

    if (flags & AUG_CREAT) {

        va_list args;
        assert(AUG_CREAT == (flags & AUG_CREAT));
        va_start(args, flags);
        mode = va_arg(args, int);
        va_end(args);

    } else
        mode = 0;

    if (!(seq = aug_openseq_(mpool, path, flags & ~AUG_TRUNC, mode,
                             sizeof(struct impl_))))
        return NULL;

    if (AUG_ISFAIL(init_(seq, &info))) {
        aug_destroyseq_(seq);
        return NULL;
    }

    impl = (struct impl_*)aug_seqtail_(seq);

    impl->mar_.vtbl_ = &marvtbl_;
    impl->mar_.impl_ = NULL;

    impl->blob_.vtbl_ = &blobvtbl_;
    impl->blob_.impl_ = NULL;

    impl->refs_ = 1;
    impl->mpool_ = mpool;

    impl->seq_ = seq;
    memcpy(&impl->info_, &info, sizeof(info));
    impl->flags_ = flags;
    impl->offset_ = 0;

    aug_retain(mpool);

    /* Fully constructed at this point. */

    if (flags & AUG_TRUNC) {

        assert(AUG_TRUNC == (flags & AUG_TRUNC));
        if (AUG_ISFAIL(aug_truncatemar(&impl->mar_, 0))) {
            aug_release(&impl->mar_);
            return NULL;
        }
    }

    return &impl->mar_;
}
