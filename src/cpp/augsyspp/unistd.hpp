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
        verify(aug_close(ref.get()));
    }

    inline std::pair<smartfd, smartfd>
    pipe()
    {
        int fds[2];
        verify(aug_pipe(fds));
        return std::make_pair(smartfd::attach(fds[0]),
                              smartfd::attach(fds[1]));
    }

    inline size_t
    read(fdref ref, void* buf, size_t size)
    {
        return verify(aug_read(ref.get(), buf, size));
    }

    inline size_t
    write(fdref ref, const void* buf, size_t size)
    {
        return verify(aug_write(ref.get(), buf, size));
    }

    inline void
    sleep(unsigned ms)
    {
        return aug_sleep(ms);
    }
}

#endif // AUGSYSPP_UNISTD_HPP
