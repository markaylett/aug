/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/body_.h"

#include "augmar/info_.h"

#include <string.h>

#if !defined(_WIN32)
# include <strings.h>
#else /* _WIN32 */
# include "augsys/windows.h"
#endif /* _WIN32 */

static void*
resize_(aug_seq_t seq, struct aug_info_* info, size_t bsize, int trunct)
{
    aug_byte_t* addr;
    if (-1 == aug_setregion_(seq, AUG_BODY(info->hsize_), info->bsize_))
        return NULL;

    if (info->bsize_ == bsize || (info->bsize_ > bsize && !trunct))
        return aug_seqaddr_(seq);

    if (!(addr = aug_resizeseq_(seq, bsize)))
        return NULL;

    if (info->bsize_ < bsize)
        bzero(addr + info->bsize_, bsize - info->bsize_);

    info->bsize_ = bsize;
    return addr;
}

AUGMAR_EXTERN int
aug_setcontent_(aug_seq_t seq, struct aug_info_* info, const void* data,
                size_t size)
{
    aug_byte_t* addr = resize_(seq, info, size, 1);
    if (!addr)
        return -1;

    memcpy(addr, data, size);
    return 0;
}

AUGMAR_EXTERN int
aug_truncate_(aug_seq_t seq, struct aug_info_* info, size_t size)
{
    return resize_(seq, info, size, 1) ? 0 : -1;
}

AUGMAR_EXTERN ssize_t
aug_write_(aug_seq_t seq, struct aug_info_* info, size_t offset,
           const void* buf, size_t size)
{
    aug_byte_t* addr = resize_(seq, info, offset + size, 0);
    if (!addr)
        return -1;

    memcpy(addr + offset, buf, size);
    return (ssize_t)size;
}

AUGMAR_EXTERN const void*
aug_content_(aug_seq_t seq, const struct aug_info_* info)
{
    if (-1 == aug_setregion_(seq, AUG_BODY(info->hsize_), info->bsize_))
        return NULL;

    return aug_seqaddr_(seq);
}

AUGMAR_EXTERN ssize_t
aug_read_(aug_seq_t seq, const struct aug_info_* info, size_t offset,
          void* buf, size_t size)
{
    size_t bsize = info->bsize_;
    const aug_byte_t* addr;

    /* If there are no bytes to read, or the offset is either at, or passed the
       end of file. */

    if (0 == size || bsize <= offset)
        return 0;

    bsize -= offset;
    if (bsize < size)
        size = bsize;

    if (!(addr = aug_content_(seq, info)))
        return -1;

    memcpy(buf, addr + offset, size);
    return (ssize_t)size;
}
