/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/seq_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/format_.h"
#include "augmar/mfile_.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/log.h"

#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
# include <strings.h>
#else /* _WIN32 */
# include "augsys/windows.h"
#endif /* _WIN32 */

struct impl_ {
    int (*destroy_)(aug_seq_t);
    void* (*resize_)(aug_seq_t, unsigned, unsigned);
    int (*sync_)(aug_seq_t);
    void* (*addr_)(aug_seq_t);
    aug_mpool* (*mpool_)(aug_seq_t);
    unsigned (*size_)(aug_seq_t);
    void* (*tail_)(aug_seq_t);
};

struct aug_seq_ {
    unsigned offset_, len_;
    const struct impl_* impl_;
};

struct memseq_ {
    struct aug_seq_ seq_;
    aug_mpool* mpool_;
    void* addr_;
    unsigned len_;
};

struct mfileseq_ {
    struct aug_seq_ seq_;
    aug_mfile_t mfile_;
};

static int
destroymem_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    aug_mpool* mpool = memseq->mpool_;
    if (memseq->addr_)
        aug_freemem(mpool, memseq->addr_);
    aug_freemem(mpool, memseq);
    aug_release(mpool);
    return 0;
}

static void*
resizemem_(aug_seq_t seq, unsigned size, unsigned tail)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    void* addr;

    if (!memseq->addr_) {

        if (!(addr = aug_callocmem(memseq->mpool_, size, 1)))
            return NULL;

        memseq->addr_ = addr;
        memseq->len_ = size;

    } else if (size > memseq->len_) {

        if (!(addr = aug_reallocmem(memseq->mpool_, memseq->addr_, size)))
            return NULL;

        bzero((char*)addr + memseq->len_, size - memseq->len_);
        memmove((char*)addr + (size - tail),
                (char*)addr + (memseq->len_ - tail), tail);

        memseq->addr_ = addr;
        memseq->len_ = size;

    } else if (size < memseq->len_) {

        memmove((char*)memseq->addr_ + (size - tail),
                (char*)memseq->addr_ + (memseq->len_ - tail), tail);

        memseq->len_ = size;
    }
    return memseq->addr_;
}

static int
syncmem_(aug_seq_t seq)
{
    return 0;
}

static void*
memaddr_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    return memseq->addr_;
}

static aug_mpool*
memmpool_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    aug_retain(memseq->mpool_);
    return memseq->mpool_;
}

static unsigned
memsize_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    return memseq->len_;
}

static void*
memtail_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    return (char*)memseq + sizeof(struct memseq_);
}

static const struct impl_ memimpl_ = {
    destroymem_,
    resizemem_,
    syncmem_,
    memaddr_,
    memmpool_,
    memsize_,
    memtail_
};

static int
destroymfile_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_closemfile_(mfileseq->mfile_);
}

static void*
resizemfile_(aug_seq_t seq, unsigned size, unsigned tail)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    unsigned len = aug_mfilesize_(mfileseq->mfile_);
    void* addr;

    if (size > len) {

        if (!(addr = aug_mapmfile_(mfileseq->mfile_, size)))
            return NULL;

        memmove((char*)addr + (size - tail),
                (char*)addr + (len - tail), tail);

        return addr;
    }

    addr = aug_mfileaddr_(mfileseq->mfile_);
    if (size < len) {

        memmove((char*)addr + (size - tail),
                (char*)addr + (len - tail), tail);

        if (-1 == aug_truncatemfile_(mfileseq->mfile_, size))
            return NULL;
    }
    return addr;
}

static int
syncmfile_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_syncmfile_(mfileseq->mfile_);
}

static void*
mfileaddr_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_mfileaddr_(mfileseq->mfile_);
}

static aug_mpool*
mfilempool_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_mfilempool_(mfileseq->mfile_);
}

static unsigned
mfilesize_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_mfilesize_(mfileseq->mfile_);
}

static void*
mfiletail_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return (char*)aug_mfiletail_(mfileseq->mfile_) + sizeof(struct mfileseq_);
}

static const struct impl_ mfileimpl_ = {
    destroymfile_,
    resizemfile_,
    syncmfile_,
    mfileaddr_,
    mfilempool_,
    mfilesize_,
    mfiletail_
};

AUG_EXTERNC void
aug_destroyseq_(aug_seq_t seq)
{
    (*seq->impl_->destroy_)(seq);
}

AUG_EXTERNC aug_result
aug_copyseq_(aug_seq_t dst, aug_seq_t src)
{
    unsigned size = aug_seqsize_(src);
    aug_result result;
    void* addr;

    if ((result = aug_setregion_(src, 0, size)) < 0)
        return result;

    if ((result = aug_setregion_(dst, 0, aug_seqsize_(dst))) < 0)
        return result;

    if (!(addr = aug_resizeseq_(dst, size)))
        return AUG_FAILERROR;

    memcpy(addr, aug_seqaddr_(src), size);
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_seq_t
aug_createseq_(aug_mpool* mpool, unsigned tail)
{
    struct memseq_* memseq;
    if (!(memseq = aug_allocmem(mpool, sizeof(struct memseq_) + tail)))
        return NULL;

    memseq->seq_.offset_ = 0;
    memseq->seq_.len_ = 0;
    memseq->seq_.impl_ = &memimpl_;
    memseq->mpool_ = mpool;
    memseq->addr_ = NULL;
    memseq->len_ = 0;
    aug_retain(mpool);
    return (aug_seq_t)memseq;
}

AUG_EXTERNC aug_seq_t
aug_openseq_(aug_mpool* mpool, const char* path, int flags, mode_t mode,
             unsigned tail)
{
    unsigned size;
    struct mfileseq_* mfileseq;
    struct aug_mfile_* mfile = aug_openmfile_(mpool, path, flags, mode,
                                              sizeof(struct mfileseq_)
                                              + tail);
    if (!mfile)
        return NULL;

    size = aug_mfileresvd_(mfile);
    if (size && !aug_mapmfile_(mfile, size))
        goto fail;

    mfileseq = (struct mfileseq_*)aug_mfiletail_(mfile);
    mfileseq->seq_.offset_ = 0;
    mfileseq->seq_.len_ = 0;
    mfileseq->seq_.impl_ = &mfileimpl_;
    mfileseq->mfile_ = mfile;

    return (aug_seq_t)mfileseq;

 fail:
    aug_closemfile_(mfile);
    return NULL;
}

AUG_EXTERNC void*
aug_resizeseq_(aug_seq_t seq, unsigned size)
{
    void* addr;
    unsigned total, tail;

    total = (*seq->impl_->size_)(seq);
    tail = total - (seq->offset_ + seq->len_);

    total += (size - seq->len_);

    if (!(addr = (*seq->impl_->resize_)(seq, total, tail)))
        return NULL;

    seq->len_ = size;
    return (char*)addr + seq->offset_;
}

AUG_EXTERNC aug_result
aug_setregion_(aug_seq_t seq, unsigned offset, unsigned len)
{
    unsigned total = (*seq->impl_->size_)(seq);
    if (total < (offset + len)) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_ERANGE,
                       AUG_MSG("sequence overrun by %d bytes"),
                       (int)((offset + len) - total));
        return AUG_FAILERROR;
    }
    seq->offset_ = offset;
    seq->len_ = len;
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_result
aug_syncseq_(aug_seq_t seq)
{
    return (*seq->impl_->sync_)(seq);
}

AUG_EXTERNC void*
aug_seqaddr_(aug_seq_t seq)
{
    return (char*)(*seq->impl_->addr_)(seq) + seq->offset_;
}

AUG_EXTERNC aug_mpool*
aug_seqmpool_(aug_seq_t seq)
{
    return (*seq->impl_->mpool_)(seq);
}

AUG_EXTERNC unsigned
aug_seqsize_(aug_seq_t seq)
{
    return (*seq->impl_->size_)(seq);
}

AUG_EXTERNC void*
aug_seqtail_(aug_seq_t seq)
{
    return (*seq->impl_->tail_)(seq);
}
