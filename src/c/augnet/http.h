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
#ifndef AUGNET_HTTP_H
#define AUGNET_HTTP_H

/**
 * @file augnet/http.h
 *
 * HTTP parser.
 */

#include "augnet/config.h"

#include "augext/http.h"
#include "augext/mpool.h"

typedef struct aug_httpparser_* aug_httpparser_t;

/**
 * If aug_createhttpparser() succeeds, aug_release() will be called from
 * aug_destroyhttpparser().
 */

AUGNET_API aug_httpparser_t
aug_createhttpparser(aug_mpool* mpool, aug_httphandler* handler,
                     unsigned size);

AUGNET_API void
aug_destroyhttpparser(aug_httpparser_t parser);

AUGNET_API aug_result
aug_appendhttp(aug_httpparser_t parser, const char*, unsigned size);

AUGNET_API aug_result
aug_finishhttp(aug_httpparser_t parser);

#endif /* AUGNET_HTTP_H */
