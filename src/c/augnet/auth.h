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
#ifndef AUGNET_AUTH_H
#define AUGNET_AUTH_H

/**
 * @file augnet/auth.h
 *
 * Digest-based authentication.
 */

#include "augnet/config.h"

#include "augutil/md5.h"

AUGNET_API char*
aug_digestha1(const char* alg, const char* username, const char* realm,
              const char* password, const char* nonce, const char* cnonce,
              aug_md5base64_t base64);

AUGNET_API char*
aug_digestresponse(const aug_md5base64_t ha1, const char* nonce,
                   const char* nc, const char* cnonce, const char* qop,
                   const char* method, const char* uri,
                   const aug_md5base64_t hentity, aug_md5base64_t base64);

#endif /* AUGNET_AUTH_H */
