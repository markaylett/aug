/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#ifndef AUGMARPP_TYPES_HPP
#define AUGMARPP_TYPES_HPP

#include "augmarpp/config.hpp"

#include "augsyspp/null.hpp"

#include "augmar/mar.h"
#include "augmar/types.h"

#include <cstring> // strlen()

namespace aug {

    class marref {
        aug_mar_t mar_;
    public:
        marref(const null_&) AUG_NOTHROW
            : mar_(0)
        {
        }
        marref(aug_mar_t mar) AUG_NOTHROW
            : mar_(mar)
        {
        }
        aug_mar_t
        get() const AUG_NOTHROW
        {
            return mar_;
        }
    };

    inline bool
    operator ==(marref lhs, marref rhs)
    {
        return lhs.get() == rhs.get();
    }
    inline bool
    operator !=(marref lhs, marref rhs)
    {
        return lhs.get() != rhs.get();
    }

    class field {
        aug_field field_;
        void
        clear()
        {
            field_.name_ = 0;
            field_.value_ = 0;
            field_.size_ = 0;
        }
    public:
        field(const null_&) AUG_NOTHROW
        {
            clear();
        }
        explicit
        field(const aug_field& f)
            : field_(f)
        {
        }
        explicit
        field(const char* s)
        {
            setname(s);
            field_.value_ = 0;
            field_.size_ = 0;
        }
        field(const char* s, const void* v, unsigned n)
        {
            setname(s);
            setvalue(v, n);
        }
        field(const char* s, const char* v)
        {
            setname(s);
            setvalue(v);
        }
        field&
        operator =(const null_&) AUG_NOTHROW
        {
            clear();
            return *this;
        }
        void
        setname(const char* s)
        {
            field_.name_ = s;
        }
        void
        setvalue(const void* v, unsigned n)
        {
            field_.value_ = v;
            field_.size_ = n;
        }
        void
        setvalue(const char* v)
        {
            field_.value_ = v;
            field_.size_ = (unsigned)strlen(v);
        }
        operator aug_field&()
        {
            return field_;
        }
        const char*
        name() const
        {
            return field_.name_;
        }
        const void*
        value() const
        {
            return field_.value_;
        }
        unsigned
        size() const
        {
            return field_.size_;
        }
        operator const aug_field&() const
        {
            return field_;
        }
    };
}

inline bool
isnull(aug_mar_t mar)
{
    return 0 == mar;
}

inline bool
isnull(aug::marref ref)
{
    return 0 == ref.get();
}

inline bool
isnull(const aug::field& f)
{
    return 0 == f.name();
}

#endif // AUGMARPP_TYPES_HPP
