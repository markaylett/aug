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
#define AUGUTIL_BUILD
#include "augutil/xstr.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/unistd.h" /* aug_fread() */

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <string.h>        /* memmove() */

/* The string's content is initially stored in the local buffer.  If the
   string grows beyond this size, a new buffer is allocated.  The pointer
   member will either point to the local buffer or the dynamically allocated
   buffer.  The local buffer is an optimisation that avoids multiple
   allocations is many cases. */

struct aug_xstr_ {
    aug_mpool* mpool_;
	size_t len_, size_;
    char* ptr_;
	char local_[1];
};

#define MINSIZE_ 64
#define AVAIL_(x) ((x)->size_ - (x)->len_)
#define ISLOCAL_(x) ((x)->ptr_ == (x)->local_)
#define TOTAL_(x) (sizeof(struct aug_xstr_) + (x)->size_)

static aug_result
resize_(aug_xstr_t xstr, size_t size)
{
    if (ISLOCAL_(xstr)) {

        /* Switch from local buffer to dynamically allocated one. */

        char* ptr = aug_allocmem(xstr->mpool_, size + 1);
        if (!ptr)
            return AUG_FAILERROR;

        /* Copy existing content from local to dynamic. */

        memcpy(ptr, xstr->ptr_, size);
        xstr->ptr_ = ptr;

    } else {

        /* Already using dynamically allocated buffer. */

        char* ptr = aug_reallocmem(xstr->mpool_, xstr->ptr_, size + 1);
        if (!ptr)
            return AUG_FAILERROR;
    }

	xstr->size_ = size;
	return AUG_SUCCESS;
}

static aug_result
reserve_(aug_xstr_t xstr, size_t size)
{
	size_t min;
	if (size <= AVAIL_(xstr))
		return AUG_SUCCESS;

    /* Grow string. */

	min = xstr->size_ * 2;
	return resize_(xstr, AUG_MAX(min, size));
}

/* This helper function allows the src and destination to be the same string.
   The length of the src string is specified so that the function can be
   re-used by strcpy-like calls, where the length is reset to zero. */

static aug_result
xstrcat_(aug_xstr_t xstr, const aug_xstr_t src, size_t len)
{
	if (!len)
		return AUG_SUCCESS;

	aug_verify(reserve_(xstr, xstr->len_ + len));

    /* Allow copy from overlapping region. */

	memmove(xstr->ptr_ + xstr->len_, src->ptr_, len);
	xstr->len_ += len;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_xstr_t
aug_createxstr(aug_mpool* mpool, size_t size)
{
    aug_xstr_t xstr;

	size = AUG_MAX(MINSIZE_, size);

    /* The initial allocation extends the local buffer.  The header already
       contains one character in the buffer, which is why there is no need for
       an extra null-terminating character. */

    if (!(xstr = aug_allocmem(mpool, sizeof(struct aug_xstr_) + size)))
		return NULL;

    xstr->mpool_ = mpool;
	xstr->len_ = 0;
	xstr->size_ = size;
    xstr->ptr_ = xstr->local_;

    aug_retain(mpool);
	return xstr;
}

AUGUTIL_API void
aug_destroyxstr(aug_xstr_t xstr)
{
    aug_mpool* mpool = xstr->mpool_;
    if (!ISLOCAL_(xstr))
        aug_freemem(mpool, xstr->ptr_);
    aug_freemem(mpool, xstr);
    aug_release(mpool);
}

AUGUTIL_API aug_result
aug_clearxstrn(aug_xstr_t xstr, size_t len)
{
    if (len < xstr->len_)
        xstr->len_ = len;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_clearxstr(aug_xstr_t xstr)
{
    return aug_clearxstrn(xstr, 0);
}

AUGUTIL_API aug_result
aug_xstrcatsn(aug_xstr_t xstr, const char* src, size_t len)
{
	if (!len)
		return AUG_SUCCESS;

	aug_verify(reserve_(xstr, xstr->len_ + len));

    /* Allow copy from overlapping region. */

	memmove(xstr->ptr_ + xstr->len_, src, len);
	xstr->len_ += len;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_xstrcats(aug_xstr_t xstr, const char* src)
{
	return aug_xstrcatsn(xstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API aug_result
aug_xstrcat(aug_xstr_t xstr, const aug_xstr_t src)
{
	return xstrcat_(xstr, src, src->len_);
}

AUGUTIL_API aug_result
aug_xstrcpysn(aug_xstr_t xstr, const char* src, size_t len)
{
    aug_verify(aug_clearxstr(xstr));
	return aug_xstrcatsn(xstr, src, len);
}

AUGUTIL_API aug_result
aug_xstrcpys(aug_xstr_t xstr, const char* src)
{
	return aug_xstrcpysn(xstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API aug_result
aug_xstrcpy(aug_xstr_t xstr, const aug_xstr_t src)
{
    /* Preserve length prior to resetting. */

    size_t len = src->len_;
    aug_verify(aug_clearxstr(xstr));

	return xstrcat_(xstr, src, len);
}

AUGUTIL_API aug_result
aug_xstrcatcn(aug_xstr_t xstr, char ch, size_t num)
{
    aug_verify(reserve_(xstr, xstr->len_ + num));

	if (1 == num)
        xstr->ptr_[xstr->len_] = ch;
	else
		memset(xstr->ptr_ + xstr->len_, ch, num);

	xstr->len_ += num;
	return AUG_SUCCESS;
}

AUGUTIL_API aug_result
aug_xstrcatc(aug_xstr_t xstr, char ch)
{
	return aug_xstrcatcn(xstr, ch, 1);
}

AUGUTIL_API aug_result
aug_xstrcpycn(aug_xstr_t xstr, char ch, size_t num)
{
    aug_verify(aug_clearxstr(xstr));
	return aug_xstrcatcn(xstr, ch, num);
}

AUGUTIL_API aug_result
aug_xstrcpyc(aug_xstr_t xstr, char ch)
{
	return aug_xstrcpycn(xstr, ch, 1);
}

AUGUTIL_API aug_rsize
aug_xstrread(aug_xstr_t xstr, aug_stream* src, size_t size)
{
    aug_rsize rsize = reserve_(xstr, xstr->len_ + size);

	if (AUG_ISFAIL(rsize))
		return rsize;

    if (AUG_ISFAIL(rsize = aug_read(src, xstr->ptr_ + xstr->len_, size)))
        return rsize;

	xstr->len_ += AUG_RESULT(rsize);
	return rsize;
}

AUGUTIL_API size_t
aug_xstrlen(aug_xstr_t xstr)
{
	return xstr->len_;
}

AUGUTIL_API const char*
aug_xstr(aug_xstr_t xstr)
{
	xstr->ptr_[xstr->len_] = '\0';
	return xstr->ptr_;
}
