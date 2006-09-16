/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
 * \file mar.hpp
 * \brief TODO
 */

#ifndef AUGMARPP_MAR_HPP
#define AUGMARPP_MAR_HPP

#include "augmarpp/smartmar.hpp"

#include "augsyspp/exception.hpp"

#include <utility> // pair<>

namespace aug {

    inline bool
    result(int ret, const char* s)
    {
        if (-1 == ret)
            throwerrinfo(s);
        return AUG_RETNOMATCH != ret;
    }
    inline void
    copymar(marref dst, marref src)
    {
        if (-1 == aug_copymar(dst.get(), src.get()))
            throwerrinfo("aug_copymar() failed");
    }
    inline smartmar
    createmar()
    {
        aug_mar_t m(aug_createmar());
        if (!m)
            throwerrinfo("aug_createmar() failed");

        return smartmar::attach(m);
    }
    inline smartmar
    openmar(const char* path, int flags)
    {
        aug_mar_t m(aug_openmar(path, flags));
        if (!m)
            throwerrinfo("aug_openmar() failed");
        return smartmar::attach(m);
    }
    inline smartmar
    openmar(const char* path, int flags, mode_t mode)
    {
        aug_mar_t m(aug_openmar(path, flags, mode));
        if (!m)
            throwerrinfo("aug_openmar() failed");
        return smartmar::attach(m);
    }
    inline void
    releasemar(aug_mar_t m)
    {
        if (-1 == aug_releasemar(m))
            throwerrinfo("aug_releasemar() failed");
    }
    inline void
    retainmar(aug_mar_t m)
    {
        if (-1 == aug_retainmar(m))
            throwerrinfo("aug_retainmar() failed");
    }
    inline void
    compactmar(marref ref)
    {
        if (-1 == aug_compactmar(ref.get()))
            throwerrinfo("aug_compactmar() failed");
    }
    inline void
    removefields(marref ref)
    {
        if (-1 == aug_removefields(ref.get()))
            throwerrinfo("aug_removefields() failed");
    }
    inline unsigned
    setfield(marref ref, const struct aug_field& f)
    {
        unsigned ord;
        if (-1 == aug_setfield(ref.get(), &f, &ord))
            throwerrinfo("aug_setfield() failed");
        return ord;
    }
    inline void
    setfield(marref ref, unsigned ord, const void* cdata, unsigned size)
    {
        if (-1 == aug_setvalue(ref.get(), ord, cdata, size))
            throwerrinfo("aug_setvalue() failed");
    }
    inline void
    setfield(marref ref, unsigned ord, const char* cdata)
    {
        setfield(ref, ord, cdata, strlen(cdata));
    }
    inline std::pair<unsigned, bool>
    unsetfield(marref ref, const char* name)
    {
        unsigned ord;
        bool match(result(aug_unsetbyname(ref.get(), name, &ord),
                          "aug_unsetbyname() failed"));
        return std::make_pair(ord, match);
    }
    inline bool
    unsetfield(marref ref, unsigned ord)
    {
        return result(aug_unsetbyord(ref.get(), ord),
                      "aug_unsetbyord() failed");
    }
    inline const void*
    getfield(marref ref, const char* name)
    {
        const void* ret = aug_valuebyname(ref.get(), name, NULL);
        if (!ret)
            throwerrinfo("aug_valuebyname() failed");
        return ret;
    }
    inline const void*
    getfield(marref ref, const char* name, unsigned& size)
    {
        const void* ret = aug_valuebyname(ref.get(), name, &size);
        if (!ret)
            throwerrinfo("aug_valuebyname() failed");
        return ret;
    }
    inline const void*
    getfield(marref ref, unsigned ord, unsigned& size)
    {
        const void* ret = aug_valuebyord(ref.get(), ord, &size);
        if (!ret)
            throwerrinfo("aug_valuebyord() failed");
        return ret;
    }
    inline const void*
    getfield(marref ref, unsigned ord)
    {
        const void* ret = aug_valuebyord(ref.get(), ord, NULL);
        if (!ret)
            throwerrinfo("aug_valuebyord() failed");
        return ret;
    }
    inline bool
    getfield(marref ref, struct aug_field& f, unsigned ord)
    {
        return result(aug_getfield(ref.get(), &f, ord),
                      "aug_getfield() failed");
    }
    inline unsigned
    getfields(marref ref)
    {
        unsigned size;
        if (-1 == aug_getfields(ref.get(), &size))
            throwerrinfo("aug_getfields() failed");
        return size;
    }
    inline bool
    toname(marref ref, const char*& s, unsigned ord)
    {
        return result(aug_ordtoname(ref.get(), &s, ord),
                      "aug_ordtoname() failed");
    }
    inline std::pair<unsigned, bool>
    toord(marref ref, const char* name)
    {
        unsigned ord;
        bool match(result(aug_nametoord(ref.get(), &ord, name),
                          "aug_nametoord() failed"));
        return std::make_pair(ord, match);
    }
    inline void
    insertmar(marref ref, const char* path)
    {
        if (-1 == aug_insertmar(ref.get(), path))
            throwerrinfo("aug_insertmar() failed");
    }
    inline off_t
    seekmar(marref ref, off_t offset, int whence)
    {
        off_t ret(aug_seekmar(ref.get(), offset, whence));
        if (-1 == ret)
            throwerrinfo("aug_seekmar() failed");
        return ret;
    }
    inline void
    setcontent(marref ref, const void* cdata, unsigned size)
    {
        if (-1 == aug_setcontent(ref.get(), cdata, size))
            throwerrinfo("aug_setcontent() failed");
    }
    inline void
    setcontent(marref ref, const char* data)
    {
        setcontent(ref, data, strlen(data));
    }
    inline void
    syncmar(marref ref)
    {
        if (-1 == aug_syncmar(ref.get()))
            throwerrinfo("aug_syncmar() failed");
    }
    inline void
    truncatemar(marref ref, unsigned size)
    {
        if (-1 == aug_truncatemar(ref.get(), size))
            throwerrinfo("aug_truncatemar() failed");
    }
    inline unsigned
    writemar(marref ref, const void* buf, unsigned size)
    {
        int ret(aug_writemar(ref.get(), buf, size));
        if (-1 == ret)
            throwerrinfo("aug_writemar() failed");
        return ret;
    }
    inline void
    extractmar(marref ref, const char* path)
    {
        if (-1 == aug_extractmar(ref.get(), path))
            throwerrinfo("aug_extractmar() failed");
    }
    inline const void*
    content(marref ref, unsigned& size)
    {
        const void* ret = aug_content(ref.get(), &size);
        if (!ret)
            throwerrinfo("aug_content() failed");
        return ret;
    }
    inline const void*
    content(marref ref)
    {
        const void* ret = aug_content(ref.get(), NULL);
        if (!ret)
            throwerrinfo("aug_content() failed");
        return ret;
    }
    inline unsigned
    readmar(marref ref, void* buf, unsigned size)
    {
        int ret(aug_readmar(ref.get(), buf, size));
        if (-1 == ret)
            throwerrinfo("aug_readmar() failed");
        return ret;
    }
    inline unsigned
    contentsize(marref ref)
    {
        unsigned size;
        if (-1 == aug_contentsize(ref.get(), &size))
            throwerrinfo("aug_contentsize() failed");
        return size;
    }
}

#endif // AUGMARPP_MAR_HPP
