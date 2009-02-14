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
#ifndef AUGCTX_TLS_H_
#define AUGCTX_TLS_H_

/**
 * @file augctx/tls_.h
 *
 * Thread-local storage.
 *
 * Functions in this module may set errno, but never errinfo.
 */

#include "augctx/config.h"
#include "augtypes.h"

#if !defined(_WIN32)
# include <pthread.h>
typedef pthread_key_t aug_tlskey_t;
#else /* _WIN32 */
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h>
typedef DWORD aug_tlskey_t;
#endif /* _WIN32 */

AUG_EXTERNC aug_bool
aug_createtlskey_(aug_tlskey_t* tlskey);

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey);

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value);

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey);

#endif /* AUGCTX_TLS_H_ */
