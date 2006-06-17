/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/strbuf.h"

static const char rcsid[] = "$Id:$";

#include "augsys/defs.h" /* AUG_MAX */
#include "augsys/errinfo.h"

#include <errno.h>       /* ENOMEM */
#include <stdlib.h>      /* malloc() */
#include <string.h>      /* memcpy() */

/* The character that forms part of the string's header is used to ensure
   that there is always enough room for the null-terminating character. */

struct aug_strbuf_ {
	size_t size_, len_;
	char data_[1];
};

#define MINSIZE_ 64
#define AVAIL_(x) ((x)->size_ - (x)->len_)
#define TOTAL_(x) (sizeof(struct aug_strbuf_) + (x)->size_)

static int
resize_(aug_strbuf_t* strbuf, size_t size)
{
	aug_strbuf_t local = realloc(*strbuf, sizeof(struct aug_strbuf_) + size);
    if (!local) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return -1;
    }

	local->size_ = size;
	*strbuf = local;
	return 0;
}

static int
reserve_(aug_strbuf_t* strbuf, size_t size)
{
	size_t min;
	if (size <= AVAIL_(*strbuf))
		return 0;

	min = (*strbuf)->size_ * 2;
	if (-1 == resize_(strbuf, AUG_MAX(min, size)))
		return -1;

	return 0;
}

AUGUTIL_API aug_strbuf_t
aug_createstrbuf(size_t size)
{
    aug_strbuf_t strbuf;

	size = AUG_MAX(MINSIZE_, size);

	if (!(strbuf = malloc(sizeof(struct aug_strbuf_) + size))) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
		return NULL;
    }

	strbuf->size_ = size;
	strbuf->len_ = 0;

	return strbuf;
}

AUGUTIL_API int
aug_freestrbuf(aug_strbuf_t strbuf)
{
	if (strbuf)
		free(strbuf);
    return 0;
}

AUGUTIL_API int
aug_clearstrbuf(aug_strbuf_t* strbuf)
{
	(*strbuf)->len_ = 0;
	return 0;
}

AUGUTIL_API int
aug_catstrbufsn(aug_strbuf_t* strbuf, const char* src, size_t len)
{
	if (!len)
		return 0;

	if (-1 == reserve_(strbuf, (*strbuf)->len_ + len))
		return -1;

	memcpy((*strbuf)->data_ + (*strbuf)->len_, src, len);
	(*strbuf)->len_ += len;
	return 0;
}

AUGUTIL_API int
aug_catstrbufs(aug_strbuf_t* strbuf, const char* src)
{
	return aug_catstrbufsn(strbuf, src, src ? strlen(src) : 0);
}

AUGUTIL_API int
aug_catstrbuf(aug_strbuf_t* strbuf, const aug_strbuf_t src)
{
	return aug_catstrbufsn(strbuf, src->data_, src->len_);
}

AUGUTIL_API int
aug_setstrbufsn(aug_strbuf_t* strbuf, const char* src, size_t len)
{
	if (-1 == aug_clearstrbuf(strbuf))
		return -1;

	return aug_catstrbufsn(strbuf, src, len);
}

AUGUTIL_API int
aug_setstrbufs(aug_strbuf_t* strbuf, const char* src)
{
	return aug_setstrbufsn(strbuf, src, src ? strlen(src) : 0);
}

AUGUTIL_API int
aug_setstrbuf(aug_strbuf_t* strbuf, const aug_strbuf_t src)
{
	return aug_setstrbufsn(strbuf, src->data_, src->len_);
}

AUGUTIL_API int
aug_catstrbufcn(aug_strbuf_t* strbuf, char ch, size_t num)
{
	if (-1 == reserve_(strbuf, (*strbuf)->len_ + num))
		return -1;

	if (1 == num)
        (*strbuf)->data_[(*strbuf)->len_] = ch;
	else
		memset((*strbuf)->data_ + (*strbuf)->len_, ch, num);

	(*strbuf)->len_ += num;
	return 0;
}

AUGUTIL_API int
aug_catstrbufc(aug_strbuf_t* strbuf, char ch)
{
	return aug_catstrbufcn(strbuf, ch, 1);
}

AUGUTIL_API int
aug_setstrbufcn(aug_strbuf_t* strbuf, char ch, size_t num)
{
	if (-1 == aug_clearstrbuf(strbuf))
		return -1;

	return aug_catstrbufcn(strbuf, ch, num);
}

AUGUTIL_API int
aug_setstrbufc(aug_strbuf_t* strbuf, char ch)
{
	return aug_setstrbufcn(strbuf, ch, 1);
}

AUGUTIL_API size_t
aug_strbuflen(aug_strbuf_t strbuf)
{
	return strbuf->len_;
}

AUGUTIL_API const char*
aug_getstr(aug_strbuf_t strbuf)
{
	strbuf->data_[strbuf->len_] = '\0';
	return strbuf->data_;
}
