#include "augsys/errno.h"

AUGSYS_EXTERN int
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    if (0xffffffff == (*tlskey = TlsAlloc())) {
        aug_setwin32errno(GetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_EXTERN int
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    if (!TlsFree(tlskey)) {
        aug_setwin32errno(GetLastError());
        return -1;
    }

    return 0;
}

AUGSYS_EXTERN int
aug_gettlsvalue_(aug_tlskey_t tlskey, void** value)
{
    void* ret = TlsGetValue(tlskey);
    if (!ret && NO_ERROR != GetLastError()) {
        aug_setwin32errno(GetLastError());
        return -1;
    }

    *value = ret;
    return 0;
}

AUGSYS_EXTERN int
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    if (!TlsSetValue(tlskey, value)) {
        aug_setwin32errno(GetLastError());
        return -1;
    }

    return 0;
}
