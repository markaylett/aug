/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/seq_.h"

static const char rcsid[] = "$Id:$";

#include "augmar/format_.h"
#include "augmar/mfile_.h"

#include "augsys/log.h"

#include <errno.h>
#include <stdlib.h>
#include <string.h>

#if !defined(_WIN32)
# include <strings.h>
#else /* _WIN32 */
# include "augsys/windows.h"
#endif /* _WIN32 */

struct impl_ {
    int (*free_)(aug_seq_t);
    void* (*resize_)(aug_seq_t, size_t, size_t);
    int (*sync_)(aug_seq_t);
    void* (*addr_)(aug_seq_t);
    size_t (*size_)(aug_seq_t);
    void* (*tail_)(aug_seq_t);
};

struct aug_seq_ {
    size_t offset_, len_;
    const struct impl_* impl_;
};

struct memseq_ {
    struct aug_seq_ seq_;
    void* addr_;
    size_t len_;
};

struct mfileseq_ {
    struct aug_seq_ seq_;
    aug_mfile_t mfile_;
};

static int
freemem_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    if (memseq->addr_)
        free(memseq->addr_);
    free(memseq);
    return 0;
}

static void*
resizemem_(aug_seq_t seq, size_t size, size_t tail)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    void* addr;

    if (!memseq->addr_) {

        if (!(addr = calloc(size, 1)))
            return NULL;

        memseq->addr_ = addr;
        memseq->len_ = size;

    } else if (size > memseq->len_) {

        if (!(addr = realloc(memseq->addr_, size)))
            return NULL;

        bzero((aug_byte_t*)addr + memseq->len_, size - memseq->len_);
        memmove((aug_byte_t*)addr + (size - tail),
                (aug_byte_t*)addr + (memseq->len_ - tail), tail);

        memseq->addr_ = addr;
        memseq->len_ = size;

    } else if (size < memseq->len_) {

        memmove((aug_byte_t*)memseq->addr_ + (size - tail),
                (aug_byte_t*)memseq->addr_ + (memseq->len_ - tail), tail);

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

static size_t
memsize_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    return memseq->len_;
}

static void*
memtail_(aug_seq_t seq)
{
    struct memseq_* memseq = (struct memseq_*)seq;
    return (aug_byte_t*)memseq + sizeof(struct memseq_);
}

static const struct impl_ memimpl_ = {
    freemem_,
    resizemem_,
    syncmem_,
    memaddr_,
    memsize_,
    memtail_
};

static int
freemfile_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_closemfile_(mfileseq->mfile_);
}

static void*
resizemfile_(aug_seq_t seq, size_t size, size_t tail)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    size_t len = aug_mfilesize_(mfileseq->mfile_);
    void* addr;

    if (size > len) {

        if (!(addr = aug_mapmfile_(mfileseq->mfile_, size)))
            return NULL;

        memmove((aug_byte_t*)addr + (size - tail),
                (aug_byte_t*)addr + (len - tail), tail);

        return addr;
    }

    addr = aug_mfileaddr_(mfileseq->mfile_);
    if (size < len) {

        memmove((aug_byte_t*)addr + (size - tail),
                (aug_byte_t*)addr + (len - tail), tail);

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

static size_t
mfilesize_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return aug_mfilesize_(mfileseq->mfile_);
}

static void*
mfiletail_(aug_seq_t seq)
{
    struct mfileseq_* mfileseq = (struct mfileseq_*)seq;
    return (aug_byte_t*)aug_mfiletail_(mfileseq->mfile_)
        + sizeof(struct mfileseq_);
}

static const struct impl_ mfileimpl_ = {
    freemfile_,
    resizemfile_,
    syncmfile_,
    mfileaddr_,
    mfilesize_,
    mfiletail_
};

AUGMAR_EXTERN int
aug_freeseq_(aug_seq_t seq)
{
    return (*seq->impl_->free_)(seq);
}

AUGMAR_EXTERN int
aug_copyseq_(aug_seq_t dst, aug_seq_t src)
{
    size_t size = aug_seqsize_(src);
    void* addr;

    if (-1 == aug_setregion_(src, 0, size))
        return -1;

    if (-1 == aug_setregion_(dst, 0, aug_seqsize_(dst)))
        return -1;

    if (!(addr = aug_resizeseq_(dst, size)))
        return -1;

    memcpy(addr, aug_seqaddr_(src), size);
    return 0;
}

AUGMAR_EXTERN aug_seq_t
aug_createseq_(size_t tail)
{
    struct memseq_* memseq;
    if (!(memseq = (struct memseq_*)malloc(sizeof(struct memseq_) + tail)))
        return NULL;

    memseq->seq_.offset_ = 0;
    memseq->seq_.len_ = 0;
    memseq->seq_.impl_ = &memimpl_;
    memseq->addr_ = NULL;
    memseq->len_ = 0;
    return (aug_seq_t)memseq;
}

AUGMAR_EXTERN aug_seq_t
aug_openseq_(const char* path, int flags, mode_t mode,
             size_t tail)
{
    size_t size;
    struct mfileseq_* mfileseq;
    struct aug_mfile_* mfile = aug_openmfile_(path, flags, mode,
                                              sizeof(struct mfileseq_) + tail);
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

AUGMAR_EXTERN void*
aug_resizeseq_(aug_seq_t seq, size_t size)
{
    void* addr;
    size_t total, tail;

    total = (*seq->impl_->size_)(seq);
    tail = total - (seq->offset_ + seq->len_);

    total += (size - seq->len_);

    if (!(addr = (*seq->impl_->resize_)(seq, total, tail)))
        return NULL;

    seq->len_ = size;
    return (aug_byte_t*)addr + seq->offset_;
}

AUGMAR_EXTERN int
aug_setregion_(aug_seq_t seq, size_t offset, size_t len)
{
    size_t total = (*seq->impl_->size_)(seq);
    if (total < (offset + len)) {

        errno = EINVAL;
        aug_error("sequence overrun by %d bytes", (offset + len) - total);
        return -1;
    }
    seq->offset_ = offset;
    seq->len_ = len;
    return 0;
}

AUGMAR_EXTERN int
aug_syncseq_(aug_seq_t seq)
{
    return (*seq->impl_->sync_)(seq);
}

AUGMAR_EXTERN void*
aug_seqaddr_(aug_seq_t seq)
{
    return (aug_byte_t*)(*seq->impl_->addr_)(seq)
        + seq->offset_;
}

AUGMAR_EXTERN size_t
aug_seqsize_(aug_seq_t seq)
{
    return (*seq->impl_->size_)(seq);
}

AUGMAR_EXTERN void*
aug_seqtail_(aug_seq_t seq)
{
    return (*seq->impl_->tail_)(seq);
}
