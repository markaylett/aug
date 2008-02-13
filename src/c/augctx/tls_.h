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
#include "augtypes.h"

#if !defined(_WIN32)
# include <pthread.h>
typedef pthread_key_t aug_tlskey_t;
#else /* _WIN32 */
# include <windows.h>
typedef DWORD aug_tlskey_t;
#endif /* _WIN32 */

AUG_EXTERNC aug_result
aug_createtlskey_(aug_tlskey_t* tlskey);

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey);

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value);

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey);

#endif /* AUGCTX_TLS_H_ */
