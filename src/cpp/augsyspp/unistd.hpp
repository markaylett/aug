/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGSYSPP_UNISTD_HPP
#define AUGSYSPP_UNISTD_HPP

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augsys/unistd.h"

namespace aug {

    inline void
    close(fdref ref)
    {
        verify(aug_fclose(ref.get()));
    }

    inline autofd
    open(const char* path, int flags)
    {
        return autofd(verify(aug_fopen(path, flags)));
    }

    inline autofd
    open(const char* path, int flags, mode_t mode)
    {
        return autofd(verify(aug_fopen(path, flags, mode)));
    }

    inline autofds
    pipe()
    {
        aug_fd fds[2];
        verify(aug_fpipe(fds));
        return autofds(fds[0], fds[1]);
    }

    inline size_t
    read(fdref ref, void* buf, size_t size)
    {
        return verify(aug_fread(ref.get(), buf, size));
    }

    inline size_t
    write(fdref ref, const void* buf, size_t size)
    {
        return verify(aug_fwrite(ref.get(), buf, size));
    }

    inline void
    msleep(unsigned ms)
    {
        return aug_msleep(ms);
    }
}

#endif // AUGSYSPP_UNISTD_HPP
