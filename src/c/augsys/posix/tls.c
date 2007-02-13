#include <errno.h>

AUGSYS_EXTERN int
aug_createtlskey_(aug_tlskey_t* tlskey)
{
    int err = pthread_key_create(tlskey, NULL);
    if (0 != err) {
        errno = err;
        return -1;
    }
    return 0;
}

AUGSYS_EXTERN int
aug_destroytlskey_(aug_tlskey_t tlskey)
{
    int err = pthread_key_delete(tlskey);
    if (0 != err) {
        errno = err;
        return -1;
    }
    return 0;
}

AUGSYS_EXTERN int
aug_gettlsvalue_(aug_tlskey_t tlskey, void** value)
{
    /* The pthread_getspecific() function is documented not to return any
       errors. */

    *value = pthread_getspecific(tlskey);
    return 0;
}

AUGSYS_EXTERN int
aug_settlsvalue_(aug_tlskey_t tlskey, void* value)
{
    int err = pthread_setspecific(tlskey, value);
    if (0 != err) {
        errno = err;
        return -1;
    }
    return 0;
}
