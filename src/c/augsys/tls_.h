/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_TLS_H_
#define AUGSYS_TLS_H_

#include "augsys/config.h"

#if !defined(_WIN32)
# include <pthread.h>
typedef pthread_key_t aug_tlskey_t;
#else /* _WIN32 */
# include "augsys/windows.h"
typedef DWORD aug_tlskey_t;
#endif /* _WIN32 */

AUGSYS_EXTERN int
aug_createtlskey_(aug_tlskey_t* tlskey);

AUGSYS_EXTERN int
aug_freetlskey_(aug_tlskey_t tlskey);

AUGSYS_EXTERN int
aug_gettlsvalue_(aug_tlskey_t tlskey, void** value);

AUGSYS_EXTERN int
aug_settlsvalue_(aug_tlskey_t tlskey, void* value);

#endif /* AUGSYS_TLS_H_ */
