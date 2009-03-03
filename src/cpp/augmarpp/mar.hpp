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
    clearfields(marref ref)
    {
        verify(aug_clearfields(ref.get()));
    }
    inline void
    delfield(marref ref, unsigned n)
    {
        verify(aug_delfieldn(ref.get(), n));
    }
    inline unsigned
    delfield(marref ref, const char* name)
    {
        return verify(aug_delfieldp(ref.get(), name));
    }
    inline const void*
    getfield(marref ref, unsigned n, unsigned& size)
    {
        const void* cdata;
        aug_rint rint(aug_getfieldn(ref.get(), n, &cdata));
        if (AUG_ISNONE(rint))
            return NULL;
        verify(rint);
        size = AUG_RESULT(rint);
        return cdata;
    }
    inline const void*
    getfield(marref ref, unsigned n)
    {
        unsigned size;
        return getfield(ref, n, size);
    }
    inline const void*
    getfield(marref ref, const char* name, unsigned& size)
    {
        const void* cdata;
        aug_rint rint(aug_getfieldp(ref.get(), name, &cdata));
        if (AUG_ISNONE(rint))
            return NULL;
        verify(rint);
        size = AUG_RESULT(rint);
        return cdata;
    }
    inline const void*
    getfield(marref ref, const char* name)
    {
        unsigned size;
        return getfield(ref, name, size);
    }
    inline bool
    getfield(marref ref, unsigned n, aug_field& f)
    {
        aug_result result(aug_getfield(ref.get(), n, &f));
        if (AUG_ISNONE(result))
            return false;
        verify(result);
        return true;
    }
    inline void
    putfield(marref ref, unsigned n, const void* cdata, unsigned size)
    {
        verify(aug_putfieldn(ref.get(), n, cdata, size));
    }
    inline void
    putfield(marref ref, unsigned n, const char* cdata)
    {
        putfield(ref, n, cdata, static_cast<unsigned>(strlen(cdata)));
    }
    inline void
    putfield(marref ref, const char* name, const void* cdata, unsigned size)
    {
        verify(aug_putfieldp(ref.get(), name, cdata, size));
    }
    inline void
    putfield(marref ref, const char* name, const char* cdata)
    {
        putfield(ref, name, cdata, static_cast<unsigned>(strlen(cdata)));
    }
    inline unsigned
    putfield(marref ref, const aug_field& f)
    {
        return AUG_RESULT(verify(aug_putfield(ref.get(), &f)));
    }
    inline const char*
    fieldntop(marref ref, unsigned n)
    {
        const char* name;
        verify(aug_fieldntop(ref.get(), n, &name));
        return name;
    }
    inline unsigned
    fieldpton(marref ref, const char* name)
    {
        return AUG_RESULT(verify(aug_fieldpton(ref.get(), name)));
    }
    inline unsigned
    getfieldcount(marref ref)
    {
        return aug_getfieldcount(ref.get());
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
