/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGSYSPP_UNISTD_HPP
#define AUGSYSPP_UNISTD_HPP

#include "augsyspp/smartfd.hpp"

#include "augctxpp/exception.hpp"

#include "augsys/unistd.h"

namespace aug {

    inline void
    close(fdref ref)
    {
        verify(aug_fclose(ref.get()));
    }

    /**
     * Set file non-blocking on or off.
     *
     * @param ref File descriptor.
     *
     * @param on On or off.
     */

    inline void
    setnonblock(fdref ref, bool on)
    {
        verify(aug_fsetnonblock(ref.get(), on ? 1 : 0));
    }

    inline autofd
    open(const char* path, int flags)
    {
        autofd fd(aug_fopen(path, flags), close);
        if (null == fd)
            throwerror();
        return fd;
    }

    inline autofd
    open(const char* path, int flags, mode_t mode)
    {
        autofd fd(aug_fopen(path, flags, mode), close);
        if (null == fd)
            throwerror();
        return fd;
    }

    inline autofds
    pipe()
    {
        aug_fd fds[2];
        verify(aug_fpipe(fds));
        return autofds(fds[0], fds[1], close);
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

    /**
     * Get size of file in bytes.
     *
     * @param ref File descriptor.
     *
     * @return Size in bytes.
     */

    inline size_t
    fsize(fdref ref)
    {
        size_t size;
        verify(aug_fsize(ref.get(), &size));
        return size;
    }

    inline void
    msleep(unsigned ms)
    {
        return aug_msleep(ms);
    }
}

#endif // AUGSYSPP_UNISTD_HPP
