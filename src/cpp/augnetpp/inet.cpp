/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNETPP_BUILD
#include "augnetpp/inet.hpp"

using namespace aug;
using namespace std;

AUGNETPP_API smartfd
aug::openpassive(const struct sockaddr_in& addr)
{
    smartfd sfd(smartfd::attach(aug_openpassive(&addr)));
    if (null == sfd)
        error("aug_openpassive() failed");

    return sfd;
}
