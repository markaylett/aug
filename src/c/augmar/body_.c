/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#define AUGMAR_BUILD
#include "augmar/body_.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augmar/format_.h"
#include "augmar/info_.h"

#include <string.h>

#if !defined(_WIN32)
# include <strings.h>
#else /* _WIN32 */
# include "augsys/windows.h"
#endif /* _WIN32 */

static void*
resize_(aug_seq_t seq, struct aug_info_* info, unsigned bsize, int trunct)
{
    char* addr;
    if (AUG_ISFAIL(aug_setregion_(seq, AUG_BODY(info->hsize_), info->bsize_)))
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

AUG_EXTERNC aug_result
aug_setcontent_(aug_seq_t seq, struct aug_info_* info, const void* data,
                unsigned size)
{
    char* addr = resize_(seq, info, size, 1);
    if (!addr)
        return AUG_FAILERROR;

    memcpy(addr, data, size);
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_result
aug_truncate_(aug_seq_t seq, struct aug_info_* info, unsigned size)
{
    return resize_(seq, info, size, 1) ? AUG_SUCCESS : AUG_FAILERROR;
}

AUG_EXTERNC aug_rsize
aug_write_(aug_seq_t seq, struct aug_info_* info, unsigned offset,
           const void* buf, unsigned len)
{
    char* addr = resize_(seq, info, offset + len, 0);
    if (!addr)
        return AUG_FAILERROR;

    memcpy(addr + offset, buf, len);
    return AUG_MKRESULT(len);
}

AUG_EXTERNC const void*
aug_getcontent_(aug_seq_t seq, const struct aug_info_* info)
{
    if (AUG_ISFAIL(aug_setregion_(seq, AUG_BODY(info->hsize_), info->bsize_)))
        return NULL;

    return aug_seqaddr_(seq);
}

AUG_EXTERNC aug_rsize
aug_read_(aug_seq_t seq, const struct aug_info_* info, unsigned offset,
          void* buf, unsigned len)
{
    unsigned bsize = info->bsize_;
    const char* addr;

    /* If there are no bytes to read, or the offset is either at or passed the
       end of file. */

    if (0 == len || bsize <= offset)
        return AUG_SUCCESS;

    bsize -= offset;
    if (bsize < len)
        len = bsize;

    if (!(addr = aug_getcontent_(seq, info)))
        return AUG_FAILERROR;

    memcpy(buf, addr + offset, len);
    return AUG_MKRESULT(len);
}
