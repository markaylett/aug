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
#ifndef AUGUTIL_PWD_H
#define AUGUTIL_PWD_H

/**
 * @file augutil/pwd.h
 *
 * Password functions.
 */

#include "augutil/md5.h"

#define AUG_MAXPASSWORD 128

typedef char aug_pwd_t[AUG_MAXPASSWORD + 1];

AUGUTIL_API char*
aug_getpass(const char* prompt, char* buf, size_t len);

AUGUTIL_API char*
aug_digestpass(const char* username, const char* realm,
               const char* password, aug_md5base64_t base64);

#endif /* AUGUTIL_PWD_H */
