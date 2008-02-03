/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/xstr.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"
#include "augsys/unistd.h" /* aug_read() */

#include <errno.h>         /* ENOMEM */
#include <stdlib.h>        /* malloc() */
#include <string.h>        /* memmove() */

/* The character that forms part of the string's header is used to ensure
   that there is always enough room for the null-terminating character. */

struct aug_xstr_ {
	size_t size_, len_;
	char data_[1];
};

#define MINSIZE_ 64
#define AVAIL_(x) ((x)->size_ - (x)->len_)
#define TOTAL_(x) (sizeof(struct aug_xstr_) + (x)->size_)

static int
resize_(aug_xstr_t* xstr, size_t size)
{
	aug_xstr_t local = realloc(*xstr, sizeof(struct aug_xstr_) + size);
    if (!local) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return -1;
    }

	local->size_ = size;
	*xstr = local;
	return 0;
}

static int
reserve_(aug_xstr_t* xstr, size_t size)
{
	size_t min;
	if (size <= AVAIL_(*xstr))
		return 0;

	min = (*xstr)->size_ * 2;
	if (-1 == resize_(xstr, AUG_MAX(min, size)))
		return -1;

	return 0;
}

AUGUTIL_API aug_xstr_t
aug_createxstr(size_t size)
{
    aug_xstr_t xstr;

	size = AUG_MAX(MINSIZE_, size);

	if (!(xstr = malloc(sizeof(struct aug_xstr_) + size))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
		return NULL;
    }

	xstr->size_ = size;
	xstr->len_ = 0;

	return xstr;
}

AUGUTIL_API int
aug_destroyxstr(aug_xstr_t xstr)
{
	if (xstr)
		free(xstr);
    return 0;
}

AUGUTIL_API int
aug_clearxstrn(aug_xstr_t* xstr, size_t len)
{
    if (len < (*xstr)->len_)
        (*xstr)->len_ = len;
	return 0;
}

AUGUTIL_API int
aug_clearxstr(aug_xstr_t* xstr)
{
    return aug_clearxstrn(xstr, 0);
}

AUGUTIL_API int
aug_xstrcatsn(aug_xstr_t* xstr, const char* src, size_t len)
{
	if (!len)
		return 0;

	if (-1 == reserve_(xstr, (*xstr)->len_ + len))
		return -1;

    /* Allow copy from overlapping region. */

	memmove((*xstr)->data_ + (*xstr)->len_, src, len);
	(*xstr)->len_ += len;
	return 0;
}

AUGUTIL_API int
aug_xstrcats(aug_xstr_t* xstr, const char* src)
{
	return aug_xstrcatsn(xstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API int
aug_xstrcat(aug_xstr_t* xstr, const aug_xstr_t src)
{
	return aug_xstrcatsn(xstr, src->data_, src->len_);
}

AUGUTIL_API int
aug_xstrcpysn(aug_xstr_t* xstr, const char* src, size_t len)
{
	if (-1 == aug_clearxstr(xstr))
		return -1;

	return aug_xstrcatsn(xstr, src, len);
}

AUGUTIL_API int
aug_xstrcpys(aug_xstr_t* xstr, const char* src)
{
	return aug_xstrcpysn(xstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API int
aug_xstrcpy(aug_xstr_t* xstr, const aug_xstr_t src)
{
	return aug_xstrcpysn(xstr, src->data_, src->len_);
}

AUGUTIL_API int
aug_xstrcatcn(aug_xstr_t* xstr, char ch, size_t num)
{
	if (-1 == reserve_(xstr, (*xstr)->len_ + num))
		return -1;

	if (1 == num)
        (*xstr)->data_[(*xstr)->len_] = ch;
	else
		memset((*xstr)->data_ + (*xstr)->len_, ch, num);

	(*xstr)->len_ += num;
	return 0;
}

AUGUTIL_API int
aug_xstrcatc(aug_xstr_t* xstr, char ch)
{
	return aug_xstrcatcn(xstr, ch, 1);
}

AUGUTIL_API ssize_t
aug_xstrcatf(int fd, aug_xstr_t* xstr, size_t size)
{
    ssize_t ret;
	if (-1 == reserve_(xstr, (*xstr)->len_ + size))
		return -1;

    if (-1 == (ret = aug_read(fd, (*xstr)->data_ + (*xstr)->len_, size)))
        return -1;

	(*xstr)->len_ += ret;
	return ret;
}

AUGUTIL_API int
aug_xstrcpycn(aug_xstr_t* xstr, char ch, size_t num)
{
	if (-1 == aug_clearxstr(xstr))
		return -1;

	return aug_xstrcatcn(xstr, ch, num);
}

AUGUTIL_API int
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
