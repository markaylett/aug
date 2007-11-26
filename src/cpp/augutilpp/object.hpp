/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_OBJECT_HPP
#define AUGUTILPP_OBJECT_HPP

#include "augobjpp.hpp"

#include "augutil/object.h"

namespace aug {

    const struct deleter_ { } deleter = deleter_();

    inline smartobj<aug_ptrobj>
    createptrobj(void* p, void (*destroy)(void*))
    {
        return object_attach(aug_createptrobj(p, destroy));
    }

    inline smartobj<aug_ptrobj>
    createptrobj(void* p, const deleter_&)
    {
        return createptrobj(p, ::operator delete);
    }

    template <typename T>
    T
    objtoptr(aug_object* obj)
    {
        return static_cast<T>(aug_objtoptr(obj));
    }
}

#endif // AUGUTILPP_OBJECT_HPP
