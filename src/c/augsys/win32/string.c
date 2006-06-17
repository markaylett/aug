/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/errno.h"

AUGSYS_API const char*
aug_strerror(int errnum)
{
	switch (errnum) {
	case EINPROGRESS:
		return "Operation now in progress";
	case EALREADY:
		return "Operation already in progress";
	case ENOTSOCK:
		return "Socket operation on nonsocket";
	case EDESTADDRREQ:
		return "Destination address required";
	case EMSGSIZE:
		return "Message too long";
	case EPROTOTYPE:
		return "Protocol wrong type for socket";
	case ENOPROTOOPT:
		return "Bad protocol option";
	case EPROTONOSUPPORT:
		return "Protocol not supported";
	case ESOCKTNOSUPPORT:
		return "Socket type not supported";
	case EOPNOTSUPP:
		return "Operation not supported on socket";
	case EPFNOSUPPORT:
		return "Protocol family not supported";
	case EAFNOSUPPORT:
		return "Address family not supported by protocol family";
	case EADDRINUSE:
		return "Address already in use";
	case EADDRNOTAVAIL:
		return "Cannot assign requested address";
	case ENETDOWN:
		return "Network is down";
	case ENETUNREACH:
		return "Network is unreachable";
	case ENETRESET:
		return "Net dropped connection or reset";
	case ECONNABORTED:
		return "Software caused connection abort";
	case ECONNRESET:
		return "Connection reset by peer";
	case ENOBUFS:
		return "No buffer space available";
	case EISCONN:
		return "Socket is already connected";
	case ENOTCONN:
		return "Socket is not connected";
	case ESHUTDOWN:
		return "Cannot send after socket shutdown";
	case ETOOMANYREFS:
		return "Too many references: cannot splice";
	case ETIMEDOUT:
		return "Connection timed out";
	case ECONNREFUSED:
		return "Connection refused";
	case ELOOP:
		return "Too many levels of symbolic links";
	case EHOSTDOWN:
		return "Host is down";
	case EHOSTUNREACH:
		return "No route to host";
	case EPROCLIM:
		return "Too many processes";
	case EUSERS:
		return "Too many users";
	case EDQUOT:
		return "Disk quota exceeded";
	case ESTALE:
		return "Stale NFS file handle";
	case EREMOTE:
		return "Too many levels of remote in path";
	case ESYSNOTREADY:
		return "Network subsystem is unavailable";
	case EVERNOTSUPPORTED:
		return "Winsock DLL version out of range";
	case ENOTINITIALISED:
		return "Winsock not initialized";
	case EDISCON:
		return "Disconnect";
	case EHOSTNOTFOUND:
		return "Host not found";
	case ETRYAGAIN:
		return "Nonauthoritative host not found";
	case ENORECOVERY:
		return "Nonrecoverable error";
	case ENODATA:
		return "Valid name, no data record of requested type";
    case EIOINCOMPLETE:
        return "Overlapped I/O event object not in signaled state";
    case EIOPENDING:
        return "Overlapped operations will complete later";
    case EOPERATIONABORTED:
        return "Overlapped operation aborted";
    }
    return strerror(errnum);
}
