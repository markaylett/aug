/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/xstr.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/unistd.h" /* aug_fread() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>         /* ENOMEM */
#include <string.h>        /* memmove() */

/* The character that forms part of the string's header is used to ensure that
   there is always enough room for a null-terminator. */

struct aug_xstr_ {
    aug_mpool* mpool_;
	size_t size_, len_;
	char data_[1];
};

#define MINSIZE_ 64
#define AVAIL_(x) ((x)->size_ - (x)->len_)
#define TOTAL_(x) (sizeof(struct aug_xstr_) + (x)->size_)

static aug_result
resize_(aug_xstr_t* xstr, size_t size)
{
	aug_xstr_t local = aug_reallocmem((*xstr)->mpool_, *xstr,
                                      sizeof(struct aug_xstr_) + size);
    if (!local)
        return AUG_FAILERROR;

	local->size_ = size;
	*xstr = local;
	return AUG_SUCCESS;
}

static aug_result
reserve_(aug_xstr_t* xstr, size_t size)
{
	size_t min;
	if (size <= AVAIL_(*xstr))
		return AUG_SUCCESS;

	min = (*xstr)->size_ * 2;
	if (resize_(xstr, AUG_MAX(min, size)) < 0)
		return AUG_FAILERROR;

	return AUG_SUCCESS;
}

AUGUTIL_API aug_xstr_t
aug_createxstr(aug_mpool* mpool, size_t size)
{
    aug_xstr_t xstr;

	size = AUG_MAX(MINSIZE_, size);

    if (!(xstr = aug_allocmem(mpool, sizeof(struct aug_xstr_) + size)))
		return NULL;

    xstr->mpool_ = mpool;
	xstr->size_ = size;
	xstr->len_ = 0;

    aug_retain(mpool);
	return xstr;
}

AUGUTIL_API aug_result
aug_destroyxstr(aug_xstr_t xstr)
{
    aug_mpool* mpool = xstr->mpool_;
    aug_freemem(mpool, xstr);
    aug_release(mpool);
    return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_clearxstrn(aug_xstr_t* xstr, size_t len)
{
    if (len < (*xstr)->len_)
        (*xstr)->len_ = len;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_clearxstr(aug_xstr_t* xstr)
{
    return aug_clearxstrn(xstr, 0);
}

AUGUTIL_API aug_result
aug_xstrcatsn(aug_xstr_t* xstr, const char* src, size_t len)
{
	if (!len)
		return AUG_SUCCESS;

	if (reserve_(xstr, (*xstr)->len_ + len) < 0)
		return AUG_FAILERROR;

    /* Allow copy from overlapping region. */

	memmove((*xstr)->data_ + (*xstr)->len_, src, len);
	(*xstr)->len_ += len;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_xstrcats(aug_xstr_t* xstr, const char* src)
{
	return aug_xstrcatsn(xstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API aug_result
aug_xstrcat(aug_xstr_t* xstr, const aug_xstr_t src)
{
	return aug_xstrcatsn(xstr, src->data_, src->len_);
}

AUGUTIL_API aug_result
aug_xstrcpysn(aug_xstr_t* xstr, const char* src, size_t len)
{
	if (aug_clearxstr(xstr) < 0)
		return AUG_FAILERROR;

	return aug_xstrcatsn(xstr, src, len);
}

AUGUTIL_API aug_result
aug_xstrcpys(aug_xstr_t* xstr, const char* src)
{
	return aug_xstrcpysn(xstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API aug_result
aug_xstrcpy(aug_xstr_t* xstr, const aug_xstr_t src)
{
	return aug_xstrcpysn(xstr, src->data_, src->len_);
}

AUGUTIL_API aug_result
aug_xstrcatcn(aug_xstr_t* xstr, char ch, size_t num)
{
	if (reserve_(xstr, (*xstr)->len_ + num) < 0)
		return AUG_FAILERROR;

	if (1 == num)
        (*xstr)->data_[(*xstr)->len_] = ch;
	else
		memset((*xstr)->data_ + (*xstr)->len_, ch, num);

	(*xstr)->len_ += num;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_xstrcatc(aug_xstr_t* xstr, char ch)
{
	return aug_xstrcatcn(xstr, ch, 1);
}

AUGUTIL_API ssize_t
aug_xstrcatf(aug_fd fd, aug_xstr_t* xstr, size_t size)
{
    ssize_t ret;
	if (reserve_(xstr, (*xstr)->len_ + size) < 0)
		return AUG_FAILERROR;

    if ((ret = aug_fread(fd, (*xstr)->data_ + (*xstr)->len_, size)) < 0)
        return AUG_FAILERROR;

	(*xstr)->len_ += ret;
	return ret;
}

AUGUTIL_API aug_result
aug_xstrcpycn(aug_xstr_t* xstr, char ch, size_t num)
{
	if (aug_clearxstr(xstr) < 0)
		return AUG_FAILERROR;

	return aug_xstrcatcn(xstr, ch, num);
}

AUGUTIL_API aug_result
aug_xstrcpyc(aug_xstr_t* xstr, char ch)
{
	return aug_xstrcpycn(xstr, ch, 1);
}

AUGUTIL_API size_t
aug_xstrlen(aug_xstr_t xstr)
{
	return xstr->len_;
}

AUGUTIL_API const char*
aug_xstr(aug_xstr_t xstr)
{
	xstr->data_[xstr->len_] = '\0';
	return xstr->data_;
}
