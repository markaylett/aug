/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_STRBUF_H
#define AUGUTIL_STRBUF_H

#include "augutil/config.h"

#include "augsys/types.h" /* size_t */

typedef struct aug_strbuf_* aug_strbuf_t;

AUGUTIL_API aug_strbuf_t
aug_createstrbuf(size_t size);

AUGUTIL_API int
aug_destroystrbuf(aug_strbuf_t strbuf);

AUGUTIL_API int
aug_clearstrbuf(aug_strbuf_t* strbuf);

AUGUTIL_API int
aug_catstrbufsn(aug_strbuf_t* strbuf, const char* src, size_t len);

AUGUTIL_API int
aug_catstrbufs(aug_strbuf_t* strbuf, const char* src);

AUGUTIL_API int
aug_catstrbuf(aug_strbuf_t* strbuf, const aug_strbuf_t src);

AUGUTIL_API int
aug_setstrbufsn(aug_strbuf_t* strbuf, const char* src, size_t len);

AUGUTIL_API int
aug_setstrbufs(aug_strbuf_t* strbuf, const char* src);

AUGUTIL_API int
aug_setstrbuf(aug_strbuf_t* strbuf, const aug_strbuf_t src);

AUGUTIL_API int
aug_catstrbufcn(aug_strbuf_t* strbuf, char ch, size_t num);

AUGUTIL_API int
aug_catstrbufc(aug_strbuf_t* strbuf, char ch);

AUGUTIL_API ssize_t
aug_readstrbuf(int fd, aug_strbuf_t* strbuf, size_t size);

AUGUTIL_API int
aug_setstrbufcn(aug_strbuf_t* strbuf, char ch, size_t num);

AUGUTIL_API int
aug_setstrbufc(aug_strbuf_t* strbuf, char ch);

AUGUTIL_API size_t
aug_strbuflen(aug_strbuf_t strbuf);

AUGUTIL_API const char*
aug_getstr(aug_strbuf_t strbuf);

#endif /* AUGUTIL_STRBUF_H */
