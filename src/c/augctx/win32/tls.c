/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augctx/utility.h" /* aug_check() */

#include <stdio.h>
#include <stdlib.h>         /* abort() */

AUG_EXTERNC aug_bool
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    return 0xffffffff == (*tlskey = TlsAlloc())
        ? AUG_FALSE : AUG_TRUE;
}

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    aug_check(TlsFree(tlskey));
}

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    aug_check(TlsSetValue(tlskey, value));
}

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey)
{
    void* ret = TlsGetValue(tlskey);
    aug_check(ret || NO_ERROR == GetLastError());
    return ret;
}
