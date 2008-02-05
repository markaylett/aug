/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
AUG_EXTERNC void
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    if (0xffffffff == (*tlskey = TlsAlloc()))
        abort();
}

AUG_EXTERNC void
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    if (!TlsFree(tlskey))
        abort();
}

AUG_EXTERNC void
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    if (!TlsSetValue(tlskey, value))
        abort();
}

AUG_EXTERNC void*
aug_gettlsvalue_(aug_tlskey_t tlskey)
{
    void* ret = TlsGetValue(tlskey);
    if (!ret && NO_ERROR != GetLastError())
        abort();

    return ret;
}
