/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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
#include "augctx/defs.h" /* #AUG_WIN32 */
#include "augtypes.h"

#if !defined(AUG_WIN32)
# include <pthread.h>
typedef pthread_key_t aug_tlskey_t;
#else /* AUG_WIN32 */
# if !defined(WIN32_LEAN_AND_MEAN)
#  define WIN32_LEAN_AND_MEAN
# endif /* !WIN32_LEAN_AND_MEAN */
# include <windows.h>
typedef DWORD aug_tlskey_t;
#endif /* AUG_WIN32 */

AUG_EXTERNC aug_result
aug_createtlskey_(aug_tlskey_t* tlskey);

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey);

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value);

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey);

#endif /* AUGCTX_TLS_H_ */
