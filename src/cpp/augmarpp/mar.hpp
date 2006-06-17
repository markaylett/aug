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
            throwerror(s);
        return AUG_RETNOMATCH != ret;
    }
    inline void
    copymar(marref dst, marref src)
    {
        if (-1 == aug_copymar(dst.get(), src.get()))
            throwerror("aug_copymar() failed");
    }
    inline smartmar
    createmar()
    {
        aug_mar_t m(aug_createmar());
        if (!m)
            throwerror("aug_createmar() failed");

        return smartmar::attach(m);
    }
    inline smartmar
    openmar(const char* path, int flags)
    {
        aug_mar_t m(aug_openmar(path, flags));
        if (!m)
            throwerror("aug_openmar() failed");
        return smartmar::attach(m);
    }
    inline smartmar
    openmar(const char* path, int flags, mode_t mode)
    {
        aug_mar_t m(aug_openmar(path, flags, mode));
        if (!m)
            throwerror("aug_openmar() failed");
        return smartmar::attach(m);
    }
    inline void
    releasemar(aug_mar_t m)
    {
        if (-1 == aug_releasemar(m))
            throwerror("aug_releasemar() failed");
    }
    inline void
    retainmar(aug_mar_t m)
    {
        if (-1 == aug_retainmar(m))
            throwerror("aug_retainmar() failed");
    }
    inline void
    compactmar(marref ref)
    {
        if (-1 == aug_compactmar(ref.get()))
            throwerror("aug_compactmar() failed");
    }
    inline void
    removefields(marref ref)
    {
        if (-1 == aug_removefields(ref.get()))
            throwerror("aug_removefields() failed");
    }
    inline size_t
    setfield(marref ref, const struct aug_field& f)
    {
        size_t ord;
        if (-1 == aug_setfield(ref.get(), &f, &ord))
            throwerror("aug_setfield() failed");
        return ord;
    }
    inline void
    setfield(marref ref, size_t ord, const void* cdata, size_t size)
    {
        if (-1 == aug_setvalue(ref.get(), ord, cdata, size))
            throwerror("aug_setvalue() failed");
    }
    inline void
    setfield(marref ref, size_t ord, const char* cdata)
    {
        setfield(ref, ord, cdata, strlen(cdata));
    }
    inline std::pair<size_t, bool>
    unsetfield(marref ref, const char* name)
    {
        size_t ord;
        bool match(result(aug_unsetbyname(ref.get(), name, &ord),
                          "aug_unsetbyname() failed"));
        return std::make_pair(ord, match);
    }
    inline bool
    unsetfield(marref ref, size_t ord)
    {
        return result(aug_unsetbyord(ref.get(), ord),
                      "aug_unsetbyord() failed");
    }
    inline const void*
    getfield(marref ref, const char* name)
    {
        const void* ret = aug_valuebyname(ref.get(), name, NULL);
        if (!ret)
            throwerror("aug_valuebyname() failed");
        return ret;
    }
    inline const void*
    getfield(marref ref, const char* name, size_t& size)
    {
        const void* ret = aug_valuebyname(ref.get(), name, &size);
        if (!ret)
            throwerror("aug_valuebyname() failed");
        return ret;
    }
    inline const void*
    getfield(marref ref, size_t ord, size_t& size)
    {
        const void* ret = aug_valuebyord(ref.get(), ord, &size);
        if (!ret)
            throwerror("aug_valuebyord() failed");
        return ret;
    }
    inline const void*
    getfield(marref ref, size_t ord)
    {
        const void* ret = aug_valuebyord(ref.get(), ord, NULL);
        if (!ret)
            throwerror("aug_valuebyord() failed");
        return ret;
    }
    inline bool
    getfield(marref ref, struct aug_field& f, size_t ord)
    {
        return result(aug_field(ref.get(), &f, ord),
                      "aug_field() failed");
    }
    inline size_t
    fields(marref ref)
    {
        size_t size;
        if (-1 == aug_fields(ref.get(), &size))
            throwerror("aug_fields() failed");
        return size;
    }
    inline bool
    toname(marref ref, const char*& s, size_t ord)
    {
        return result(aug_ordtoname(ref.get(), &s, ord),
                      "aug_ordtoname() failed");
    }
    inline std::pair<size_t, bool>
    toord(marref ref, const char* name)
    {
        size_t ord;
        bool match(result(aug_nametoord(ref.get(), &ord, name),
                          "aug_nametoord() failed"));
        return std::make_pair(ord, match);
    }
    inline void
    insertmar(marref ref, const char* path)
    {
        if (-1 == aug_insertmar(ref.get(), path))
            throwerror("aug_insertmar() failed");
    }
    inline off_t
    seekmar(marref ref, off_t offset, int whence)
    {
        off_t ret(aug_seekmar(ref.get(), offset, whence));
        if (-1 == ret)
            throwerror("aug_seekmar() failed");
        return ret;
    }
    inline void
    setcontent(marref ref, const void* cdata, size_t size)
    {
        if (-1 == aug_setcontent(ref.get(), cdata, size))
            throwerror("aug_setcontent() failed");
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
            throwerror("aug_syncmar() failed");
    }
    inline void
    truncatemar(marref ref, size_t size)
    {
        if (-1 == aug_truncatemar(ref.get(), size))
            throwerror("aug_truncatemar() failed");
    }
    inline size_t
    writemar(marref ref, const void* buf, size_t size)
    {
        ssize_t ret(aug_writemar(ref.get(), buf, size));
        if (-1 == ret)
            throwerror("aug_writemar() failed");
        return ret;
    }
    inline void
    extractmar(marref ref, const char* path)
    {
        if (-1 == aug_extractmar(ref.get(), path))
            throwerror("aug_extractmar() failed");
    }
    inline const void*
    content(marref ref, size_t& size)
    {
        const void* ret = aug_content(ref.get(), &size);
        if (!ret)
            throwerror("aug_content() failed");
        return ret;
    }
    inline const void*
    content(marref ref)
    {
        const void* ret = aug_content(ref.get(), NULL);
        if (!ret)
            throwerror("aug_content() failed");
        return ret;
    }
    inline size_t
    readmar(marref ref, void* buf, size_t size)
    {
        ssize_t ret(aug_readmar(ref.get(), buf, size));
        if (-1 == ret)
            throwerror("aug_readmar() failed");
        return ret;
    }
    inline size_t
    contentsize(marref ref)
    {
        size_t size;
        if (-1 == aug_contentsize(ref.get(), &size))
            throwerror("aug_contentsize() failed");
        return size;
    }
}

#endif // AUGMARPP_MAR_HPP
