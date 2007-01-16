/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
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
# if !defined(socklen_t)
typedef int socklen_t;
# endif /* socklen_t */
# if defined(_MSC_VER) && !defined(HAVE_IPV6)
#  define HAVE_IPV6 1
# endif /* _MSC_VER && !HAVE_IPV6 */
#endif /* _WIN32 */

#define AUG_MAXADDRLEN 128

struct aug_endpoint {
    socklen_t len_;
    union {
        short family_;
        struct {
            short family_;
            unsigned short port_;
        } all_;
        struct sockaddr sa_;
        struct sockaddr_in ipv4_;
#if HAVE_IPV6
        struct sockaddr_in6 ipv6_;
#endif /* HAVE_IPV6 */
        char pad_[AUG_MAXADDRLEN];
    } un_;
};

struct aug_inetaddr {
    short family_;
    union {
        struct in_addr ipv4_;
#if HAVE_IPV6
        struct in6_addr ipv6_;
#endif /* HAVE_IPV6 */
        char pad_[16];
    } un_;
};

AUGSYS_API int
aug_socket(int domain, int type, int protocol);

AUGSYS_API int
aug_accept(int s, struct aug_endpoint* ep);

AUGSYS_API int
aug_bind(int s, const struct aug_endpoint* ep);

/**
   Remember that, for non-blocking sockets, connect() may fail with
   #EINPROGRESS.  Use poll() or select() on write-status to determine
   completion.
*/

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
aug_inetntop(const struct aug_inetaddr* src, char* dst, socklen_t len);

AUGSYS_API struct aug_inetaddr*
aug_inetpton(int af, const char* src, struct aug_inetaddr* dst);

AUGSYS_API void
aug_freeaddrinfo(struct addrinfo* res);

AUGSYS_API int
aug_getaddrinfo(const char* host, const char* serv,
                const struct addrinfo* hints, struct addrinfo** res);

AUGSYS_API int
aug_getfamily(int s);

AUGSYS_API int
aug_setreuseaddr(int s, int on);

AUGSYS_API struct aug_endpoint*
aug_setinetaddr(struct aug_endpoint* ep, const struct aug_inetaddr* addr);

AUGSYS_API struct aug_inetaddr*
aug_getinetaddr(const struct aug_endpoint* ep, struct aug_inetaddr* addr);

AUGSYS_API const struct aug_inetaddr*
aug_inetany(int af);

AUGSYS_API const struct aug_inetaddr*
aug_inetloopback(int af);

AUGSYS_API int
aug_setsockerrinfo(int s);

/**
   After a failed call to aug_accept(), this function can be used to determine
   whether the error was a result of the peer closing the connection.  This
   can occur when the passive socket is non-blocking.
*/

AUGSYS_API int
aug_acceptlost(void);

#endif /* AUGSYS_SOCKET_H */
