/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include <stdlib.h> /* abort() */

AUG_EXTERNC aug_bool
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    return 0 != pthread_key_create(tlskey, NULL)
        ? AUG_FALSE : AUG_TRUE:
}

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    if (0 != pthread_key_delete(tlskey))
        abort();
}

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    if (0 != pthread_setspecific(tlskey, value))
        abort();
}

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey)
{
    /* The pthread_getspecific() function is documented not to return any
       errors. */

    return pthread_getspecific(tlskey);
}
