/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGSYS_BUILD
#include "augsys/config.h"

static const char rcsid[] = "$Id$";

#if defined(_MSC_VER)
# pragma comment(lib, "ws2_32.lib")
# pragma comment(lib, "iphlpapi.lib")
#endif /* _MSC_VER */
