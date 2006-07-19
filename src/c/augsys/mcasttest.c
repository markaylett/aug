/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys.h"

#if 0
#include <winsock2.h>
#include <stdio.h>
#include "win32/iptypes_.h"
#include <iphlpapi.h>

static void
print_(const struct sockaddr* addr, size_t size)
{
    char buf[255];
    DWORD n = sizeof(buf);
    WSAAddressToString((struct sockaddr*)addr, size, NULL, buf, &n);
    printf("address: [%s]\n", buf);
}

static int
getipv4_(void* arg, unsigned int iface, PIP_ADAPTER_ADDRESSES adapter)
{
    struct sockaddr_in* addr = arg;
    PIP_ADAPTER_UNICAST_ADDRESS it = adapter->FirstUnicastAddress;

    /* Iteration is not strictly necessary: the addresses should already be
       filtered by AF_INET. */

    for (; it ; it = it->Next)
        if (AF_INET == it->Address.lpSockaddr->sa_family) {
            memcpy(addr, it->Address.lpSockaddr, sizeof(*addr));
            return 0;
        }

    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("no address for interface '%d'"), iface);
    return -1;
}

static int
getipv6_(void* arg, unsigned int iface, PIP_ADAPTER_ADDRESSES adapter)
{
    unsigned int* i = arg;
    PIP_ADAPTER_UNICAST_ADDRESS it = adapter->FirstUnicastAddress;

    /* Iteration is not strictly necessary: the addresses should already be
       filtered by AF_INET6. */

    for (; it ; it = it->Next)
        if (AF_INET6 == it->Address.lpSockaddr->sa_family) {
            *i = adapter->Ipv6IfIndex;
            return 0;
        }

    aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                   AUG_MSG("no address for interface '%d'"), iface);
    return -1;
}

static int
getif_(int af, unsigned int iface,
       int (*fn)(void*, unsigned int, PIP_ADAPTER_ADDRESSES), void* arg)
{
    PIP_ADAPTER_ADDRESSES list = NULL;
    ULONG len = 0, ret = 0;
    int i;

    /* The size of the buffer can be different between consecutive API calls.
       In most cases, i < 2 is sufficient; one call to get the size and one
       call to get the actual parameters.  But if one more interface is added
       or addresses are added, the call again fails with BUFFER_OVERFLOW.  So
       the number is picked slightly greater than 2. */

    for (i = 0; i < 5; i++) {

        ret = GetAdaptersAddresses(af, 0, NULL, list, &len);
        if (ERROR_BUFFER_OVERFLOW != ret)
            break;

        if (list)
            free(list);

        list = malloc(len);
        if (!list) {
            ret = GetLastError();
            break;
        }
    }

    if (NO_ERROR == ret) {

        PIP_ADAPTER_ADDRESSES it = list;
        for (i = 0; it && i < iface; it = it->Next, ++i)
            ;

        if (it)
            i = fn(arg, iface, it);
        else {
            aug_seterrinfo(__FILE__, __LINE__, AUG_SRCLOCAL, AUG_EEXIST,
                           AUG_MSG("interface '%d' does not exist"), iface);
            i = -1;
        }

    } else {
        aug_setwin32errinfo(__FILE__, __LINE__, ret);
        i = -1;
    }

    if (list)
        free(list);
    return i;
}

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    struct sockaddr_in addr;
    unsigned int i;

    aug_atexitinit(&errinfo);

    if (-1 == getif_(AF_INET, 2 == argc ? atoi(argv[1]) : 0, getipv4_,
                     &addr)) {
        aug_perrinfo("getif_() failed");
        return 1;
    }
    print_((struct sockaddr*)&addr, sizeof(addr));

    if (-1 == getif_(AF_INET6, 2 == argc ? atoi(argv[1]) : 0, getipv6_, &i)) {
        aug_perrinfo("getif_() failed");
        return 1;
    }
    printf("%d\n", i);
    return 0;
}
#endif

#include <stdio.h>
#include <string.h>

struct aug_heartbeat {

    uint8_t magic_;
    uint8_t verno_;
    char group_[32];
    char name_[32];
    uint8_t weight_;
    uint8_t family_; /* 2 = IPV4, 23 = IPV6 */
    uint16_t port_;
    union {
        uint32_t ipv4_;
        uint8_t ipv6_[16];
    } addr_;
    uint16_t status_;
    uint64_t start_;
    uint64_t last_;
    uint64_t seqno_;
    uint32_t conns_;
    char text_[64];
};

static void
fixedcpy_(char* dst, const char* src, size_t size)
{
    size_t i = aug_strlcpy(dst, src, size) + 1;
	if (i < size)
		memset(dst + i, '\0', size - i);
}

#if 0
static void
write_(char* ptr, const struct aug_heartbeat* hb)
{
    fixedcpy_(ptr, hb->group_, sizeof(hb->group_));
}
#endif

int
main(int argc, char* argv[])
{
    struct aug_errinfo errinfo;
    char test[32];
    aug_atexitinit(&errinfo);
    fixedcpy_(test, "test", sizeof(test));
    return 0;
}
