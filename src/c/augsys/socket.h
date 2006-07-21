/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYS_SOCKET_H
#define AUGSYS_SOCKET_H

#include "augsys/config.h"
#include "augsys/types.h"

#if !defined(_WIN32)
# include <sys/socket.h>
# include <netinet/in.h>
# include <arpa/inet.h>
# include <netdb.h>
#else /* _WIN32 */
# include <winsock2.h>
# include <ws2tcpip.h>
# if !defined(bzero)
#  define bzero ZeroMemory
# endif /* !bzero */
# if !defined(SHUT_RD)
#  define SHUT_RD SD_RECV
#  define SHUT_WR SD_SEND
#  define SHUT_RDWR SD_BOTH
# endif /* !SHUT_RD */
typedef int socklen_t;
#endif /* _WIN32 */

#define AUG_MAXADDRLEN 128

struct aug_endpoint {
    socklen_t len_;
    union {
        short family_;
        struct sockaddr all_;
        struct sockaddr_in ipv4_;
        struct sockaddr_in6 ipv6_;
        char pad_[AUG_MAXADDRLEN];
    } un_;
};

struct aug_ipaddr {
    short family_;
    union {
        struct in_addr ipv4_;
        struct in6_addr ipv6_;
    } un_;
};

AUGSYS_API int
aug_socket(int domain, int type, int protocol);

AUGSYS_API int
aug_accept(int s, struct aug_endpoint* ep);

AUGSYS_API int
aug_bind(int s, const struct aug_endpoint* ep);

/** Remember that, for non-blocking sockets, connect() can fail with
    EINPROGRESS.  Use poll() or select() on write-status to determine
    completion. */

AUGSYS_API int
aug_connect(int s, const struct aug_endpoint* ep);

AUGSYS_API struct aug_endpoint*
aug_getpeername(int s, struct aug_endpoint* ep);

AUGSYS_API struct aug_endpoint*
aug_getsockname(int s, struct aug_endpoint* ep);

AUGSYS_API int
aug_listen(int s, int backlog);

AUGSYS_API ssize_t
aug_recv(int s, void* buf, size_t len, int flags);

AUGSYS_API ssize_t
aug_recvfrom(int s, void* buf, size_t len, int flags,
             struct aug_endpoint* ep);

AUGSYS_API ssize_t
aug_send(int s, const void* buf, size_t len, int flags);

AUGSYS_API ssize_t
aug_sendto(int s, const void* buf, size_t len, int flags,
           const struct aug_endpoint* ep);

AUGSYS_API int
aug_getsockopt(int s, int level, int optname, void* optval,
               socklen_t* optlen);

AUGSYS_API int
aug_setsockopt(int s, int level, int optname, const void* optval,
               socklen_t optlen);

AUGSYS_API int
aug_shutdown(int s, int how);

AUGSYS_API int
aug_socketpair(int domain, int type, int protocol, int sv[2]);

AUGSYS_API char*
aug_inetntop(const struct aug_ipaddr* src, char* dst, socklen_t size);

AUGSYS_API struct aug_ipaddr*
aug_inetpton(int af, const char* src, struct aug_ipaddr* dst);

AUGSYS_API void
aug_freeaddrinfo(struct addrinfo* res);

AUGSYS_API int
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res);

AUGSYS_API int
aug_setreuseaddr(int s, int on);

AUGSYS_API int
aug_getfamily(int s);

#endif /* AUGSYS_SOCKET_H */
