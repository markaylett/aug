/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/dstr.h"

static const char rcsid[] = "$Id:$";

#include "augsys/defs.h" /* AUG_MAX */
#include "augsys/errinfo.h"

#include <errno.h>       /* ENOMEM */
#include <stdlib.h>      /* malloc() */
#include <string.h>      /* memcpy() */

/* The character that forms part of the string's header is used to ensure
   that there is always enough room for the null-terminating character. */

struct aug_dstr_ {
	size_t size_, len_;
	char data_[1];
};

#define MINSIZE_ 64
#define AVAIL_(x) ((x)->size_ - (x)->len_)
#define TOTAL_(x) (sizeof(struct aug_dstr_) + (x)->size_)

static int
resize_(aug_dstr_t* dstr, size_t size)
{
	aug_dstr_t local = realloc(*dstr, sizeof(struct aug_dstr_) + size);
    if (!local) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
        return -1;
    }

	local->size_ = size;
	*dstr = local;
	return 0;
}

static int
reserve_(aug_dstr_t* dstr, size_t size)
{
	size_t min;
	if (size <= AVAIL_(*dstr))
		return 0;

	min = (*dstr)->size_ * 2;
	if (-1 == resize_(dstr, AUG_MAX(min, size)))
		return -1;

	return 0;
}

AUGUTIL_API aug_dstr_t
aug_createdstr(size_t size)
{
    aug_dstr_t dstr;

	size = AUG_MAX(MINSIZE_, size);

	if (!(dstr = malloc(sizeof(struct aug_dstr_) + size))) {
        aug_setposixerrinfo(__FILE__, __LINE__, ENOMEM);
		return NULL;
    }

	dstr->size_ = size;
	dstr->len_ = 0;

	return dstr;
}

AUGUTIL_API int
aug_freedstr(aug_dstr_t dstr)
{
	if (dstr)
		free(dstr);
    return 0;
}

AUGUTIL_API int
aug_cleardstr(aug_dstr_t* dstr)
{
	(*dstr)->len_ = 0;
	return 0;
}

AUGUTIL_API int
aug_dstrcatsn(aug_dstr_t* dstr, const char* src, size_t len)
{
	if (!len)
		return 0;

	if (-1 == reserve_(dstr, (*dstr)->len_ + len))
		return -1;

	memcpy((*dstr)->data_ + (*dstr)->len_, src, len);
	(*dstr)->len_ += len;
	return 0;
}

AUGUTIL_API int
aug_dstrcats(aug_dstr_t* dstr, const char* src)
{
	return aug_dstrcatsn(dstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API int
aug_dstrcat(aug_dstr_t* dstr, const aug_dstr_t src)
{
	return aug_dstrcatsn(dstr, src->data_, src->len_);
}

AUGUTIL_API int
aug_dstrsetsn(aug_dstr_t* dstr, const char* src, size_t len)
{
	if (-1 == aug_cleardstr(dstr))
		return -1;

	return aug_dstrcatsn(dstr, src, len);
}

AUGUTIL_API int
aug_dstrsets(aug_dstr_t* dstr, const char* src)
{
	return aug_dstrsetsn(dstr, src, src ? strlen(src) : 0);
}

AUGUTIL_API int
aug_dstrset(aug_dstr_t* dstr, const aug_dstr_t src)
{
	return aug_dstrsetsn(dstr, src->data_, src->len_);
}

AUGUTIL_API int
aug_dstrcatcn(aug_dstr_t* dstr, char ch, size_t num)
{
	if (-1 == reserve_(dstr, (*dstr)->len_ + num))
		return -1;

	if (1 == num)
        (*dstr)->data_[(*dstr)->len_] = ch;
	else
		memset((*dstr)->data_ + (*dstr)->len_, ch, num);

	(*dstr)->len_ += num;
	return 0;
}

AUGUTIL_API int
aug_dstrcatc(aug_dstr_t* dstr, char ch)
{
	return aug_dstrcatcn(dstr, ch, 1);
}

AUGUTIL_API int
aug_dstrsetcn(aug_dstr_t* dstr, char ch, size_t num)
{
	if (-1 == aug_cleardstr(dstr))
		return -1;

	return aug_dstrcatcn(dstr, ch, num);
}

AUGUTIL_API int
aug_dstrsetc(aug_dstr_t* dstr, char ch)
{
	return aug_dstrsetcn(dstr, ch, 1);
}

AUGUTIL_API size_t
aug_dstrlen(aug_dstr_t dstr)
{
	return dstr->len_;
}

AUGUTIL_API const char*
aug_dstr(aug_dstr_t dstr)
{
	dstr->data_[dstr->len_] = '\0';
	return dstr->data_;
}
