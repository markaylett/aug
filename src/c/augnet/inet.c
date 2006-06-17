/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGNET_BUILD
#include "augnet/inet.h"

static const char rcsid[] = "$Id:$";

#include "augutil/conv.h"

#include "augsys/inet.h"   /* aug_inetaton() */
#include "augsys/socket.h"
#include "augsys/unistd.h" /* aug_close() */

#if !defined(_WIN32)
# include <alloca.h>
# include <netdb.h>        /* gethostbyname() */
# include <netinet/in.h>
# include <netinet/tcp.h>
#else /* _WIN32 */
# include <malloc.h>
#endif /* _WIN32 */

#include <errno.h>
#include <string.h>        /* strchr() */

AUGNET_API int
aug_tcplisten(const struct sockaddr_in* addr)
{
    static const int ON = 1;

    int fd = aug_socket(AF_INET, SOCK_STREAM, 0);
    if (-1 == fd)
        return -1;

    if (-1 == aug_setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &ON, sizeof(ON)))
        goto fail;

    if (-1 == aug_bind(fd, (const struct sockaddr*)addr, sizeof(*addr)))
        goto fail;

    if (-1 == aug_listen(fd, SOMAXCONN))
        goto fail;

    return fd;

 fail:
    aug_close(fd);
    return -1;
}

AUGNET_API struct sockaddr_in*
aug_parseinet(struct sockaddr_in* dst, const char* src)
{
	size_t len;
    unsigned short nport;
	char* host;

	/* Locate host and port separator. */

	const char* port = strchr(src, ':');
	if (NULL == port)
		goto fail;

	/* Calculate length of host part. */

	len = port - src;

	/* Ensure host and port parts exists. */

	if (1 > len || '\0' == *++port)
		goto fail;

    /* Parse port value. */

    if (-1 == aug_strtous(&nport, port, 10))
        goto fail;

	/* Create null-terminated host string. */

	host = alloca(len + 1);
	memcpy(host, src, len);
	host[len] = '\0';

	/* Try to resolve dotted addresss notation first. */

	if (!aug_inetaton(host, &dst->sin_addr)) {

		/* Attempt to resolve host using DNS. */

		struct hostent* answ = gethostbyname(host);
		if (!answ || !answ->h_addr_list[0])
			goto fail;

		memcpy(&dst->sin_addr, answ->h_addr_list[0],
               sizeof(dst->sin_addr));
	}

	dst->sin_family = AF_INET;
	dst->sin_port = htons(nport);
	return dst;

 fail:
	errno = EINVAL;
	return NULL;
}

AUGNET_API int
aug_setnodelay(int fd, int on)
{
    return aug_setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &on, sizeof(on));
}
