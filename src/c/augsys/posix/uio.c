/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
AUGSYS_API ssize_t
aug_readv(int fd, const struct iovec* iov, int size)
{
    return readv(fd, iov, size);
}

AUGSYS_API ssize_t
aug_writev(int fd, const struct iovec* iov, int size)
{
    return writev(fd, iov, size);
}
