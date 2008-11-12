/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTIL_PWD_H
#define AUGUTIL_PWD_H

/**
 * @file augutil/pwd.h
 *
 * Password functions.
 */

#include "augutil/md5.h"

#define AUG_MAXPASSWORD 128

AUGUTIL_API char*
aug_getpass(const char* prompt, char* buf, size_t len);

AUGUTIL_API char*
aug_digestpass(const char* username, const char* realm,
               const char* password, aug_md5base64_t base64);

#endif /* AUGUTIL_PWD_H */
