/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNET_SSL_H
#define AUGNET_SSL_H

#include "augnet/config.h"

struct aug_errinfo;

AUGNET_API void
aug_setsslerrinfo(struct aug_errinfo* errinfo, const char* file, int line,
                  unsigned long err);

AUGNET_API int
aug_setsslclient(int fd, void* ssl);

AUGNET_API int
aug_setsslserver(int fd, void* ssl);

#endif /* AUGNET_SSL_H */
