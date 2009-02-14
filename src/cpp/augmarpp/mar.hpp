/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
#ifndef AUGMARPP_MAR_HPP
#define AUGMARPP_MAR_HPP

#include "augmarpp/smartmar.hpp"

#include "augctxpp/exception.hpp"

#include <utility> // pair<>

namespace aug {

    inline void
    copymar(marref dst, marref src)
    {
        verify(aug_copymar(dst.get(), src.get()));
    }
    inline smartmar
    createmar(mpoolref mpool)
    {
        return smartmar::attach(verify(aug_createmar(mpool.get())));
    }
    inline smartmar
    openmar(mpoolref mpool, const char* path, int flags)
    {
        return smartmar::attach(verify(aug_openmar(mpool.get(), path,
                                                   flags)));
    }
    inline smartmar
    openmar(mpoolref mpool, const char* path, int flags, mode_t mode)
    {
        return smartmar::attach(verify(aug_openmar(mpool.get(), path, flags,
                                                   mode)));
    }
    inline void
    releasemar(aug_mar_t m)
    {
        aug_releasemar(m);
    }
    inline void
    retainmar(aug_mar_t m)
    {
        aug_retainmar(m);
    }
    inline void
    compactmar(marref ref)
    {
        verify(aug_compactmar(ref.get()));
    }
    inline void
    removefields(marref ref)
    {
        verify(aug_removefields(ref.get()));
    }
    inline unsigned
    setfield(marref ref, const aug_field& f)
    {
        unsigned ord;
        verify(aug_setfield(ref.get(), &f, &ord));
        return ord;
    }
    inline void
    setfield(marref ref, unsigned ord, const void* cdata, unsigned size)
    {
        verify(aug_setvalue(ref.get(), ord, cdata, size));
    }
    inline void
    setfield(marref ref, unsigned ord, const char* cdata)
    {
        setfield(ref, ord, cdata, (unsigned)strlen(cdata));
    }
    inline unsigned
    unsetfield(marref ref, const char* name)
    {
        unsigned ord;
        verify(aug_unsetbyname(ref.get(), name, &ord));
        return ord;
    }
    inline void
    unsetfield(marref ref, unsigned ord)
    {
        verify(aug_unsetbyord(ref.get(), ord));
    }
    inline const void*
    getfield(marref ref, const char* name)
    {
        return verify(aug_valuebyname(ref.get(), name, 0));
    }
    inline const void*
    getfield(marref ref, const char* name, unsigned& size)
    {
        return verify(aug_valuebyname(ref.get(), name, &size));
    }
    inline const void*
    getfield(marref ref, unsigned ord, unsigned& size)
    {
        return verify(aug_valuebyord(ref.get(), ord, &size));
    }
    inline const void*
    getfield(marref ref, unsigned ord)
    {
        return verify(aug_valuebyord(ref.get(), ord, 0));
    }
    inline void
    getfield(marref ref, aug_field& f, unsigned ord)
    {
        verify(aug_getfield(ref.get(), &f, ord));
    }
    inline unsigned
    getfields(marref ref)
    {
        return aug_getfields(ref.get());
    }
    inline void
    toname(marref ref, const char*& s, unsigned ord)
    {
        verify(aug_ordtoname(ref.get(), &s, ord));
    }
    inline unsigned
    toord(marref ref, const char* name)
    {
        unsigned ord;
        verify(aug_nametoord(ref.get(), &ord, name));
        return ord;
    }
    inline void
    insertmar(marref ref, const char* path)
    {
        verify(aug_insertmar(ref.get(), path));
    }
    inline off_t
    seekmar(marref ref, off_t offset, int whence)
    {
        return AUG_RESULT(verify(aug_seekmar(ref.get(), offset, whence)));
    }
    inline void
    setcontent(marref ref, const void* cdata, unsigned size)
    {
        verify(aug_setcontent(ref.get(), cdata, size));
    }
    inline void
    setcontent(marref ref, const char* data)
    {
        setcontent(ref, data, (unsigned)strlen(data));
    }
    inline void
    syncmar(marref ref)
    {
        verify(aug_syncmar(ref.get()));
    }
    inline void
    truncatemar(marref ref, unsigned size)
    {
        verify(aug_truncatemar(ref.get(), size));
    }
    inline unsigned
    writemar(marref ref, const void* buf, unsigned size)
    {
        return AUG_RESULT(verify(aug_writemar(ref.get(), buf, size)));
    }
    inline void
    extractmar(marref ref, const char* path)
    {
        verify(aug_extractmar(ref.get(), path));
    }
    inline const void*
    getcontent(marref ref, unsigned& size)
    {
        return verify(aug_getcontent(ref.get(), &size));
    }
    inline const void*
    getcontent(marref ref)
    {
        return verify(aug_getcontent(ref.get(), 0));
    }
    inline unsigned
    readmar(marref ref, void* buf, unsigned size)
    {
        return AUG_RESULT(verify(aug_readmar(ref.get(), buf, size)));
    }
    inline unsigned
    getcontentsize(marref ref)
    {
        return aug_getcontentsize(ref.get());
    }
}

#endif // AUGMARPP_MAR_HPP
