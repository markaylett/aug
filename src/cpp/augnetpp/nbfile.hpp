/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_NBFILE_HPP
#define AUGNETPP_NBFILE_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/object.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/nbfile.h"

#include "augutil/list.h"

#include "augctx/errno.h"
#include "augctx/log.h"

#include <memory> // auto_ptr<>

namespace aug {

    template <bool (*T)(aug::objectref, mdref, unsigned short)>
    int
    nbfilecb(aug_object* ob, aug_md md, unsigned short events) AUG_NOTHROW
    {
        try {
            return T(ob, md, events) ? 1 : 0;
        } AUG_SETERRINFOCATCH;

        /**
         * Do not remove the file unless explicitly asked to.
         */

        return 1;
    }

    template <typename T, bool (T::*U)(aug::objectref, int, unsigned short)>
    int
    nbfilememcb(aug_object* ob, aug_md md, unsigned short events) AUG_NOTHROW
    {
        try {
            return (obtoaddr<T*>(ob)->*U)(md, events) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    template <typename T>
    int
    nbfilememcb(aug_object* ob, aug_md md, unsigned short events) AUG_NOTHROW
    {
        try {
            return obtoaddr<T*>(ob)->nbfilecb(md, events) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    class nbfiles {

        aug_nbfiles_t nbfiles_;

        nbfiles(const nbfiles&);

        nbfiles&
        operator =(const nbfiles&);

    public:
        ~nbfiles() AUG_NOTHROW
        {
            if (-1 == aug_destroynbfiles(nbfiles_))
                perrinfo(aug_tlx, "aug_destroynbfiles() failed");
        }

        nbfiles()
        {
            verify(nbfiles_ = aug_createnbfiles());
        }

        operator aug_nbfiles_t()
        {
            return nbfiles_;
        }

        aug_nbfiles_t
        get()
        {
            return nbfiles_;
        }
    };

    inline void
    insertnbfile(aug_nbfiles_t nbfiles, mdref ref, aug_nbfilecb_t cb,
                 aug_object* ob)
    {
        verify(aug_insertnbfile(nbfiles, ref.get(), cb, ob));
    }

    inline void
    insertnbfile(aug_nbfiles_t nbfiles, mdref ref, aug_nbfilecb_t cb,
                 const null_&)
    {
        verify(aug_insertnbfile(nbfiles, ref.get(), cb, 0));
    }

    template <typename T>
    void
    insertnbfile(aug_nbfiles_t nbfiles, mdref ref, T& x)
    {
        aug::smartob<aug_addrob> ob(createaddrob(&x, 0));
        verify(aug_insertnbfile
               (nbfiles, ref.get(), nbfilememcb<T>, ob.base()));
    }

    template <typename T>
    void
    insertnbfile(aug_nbfiles_t nbfiles, mdref ref, std::auto_ptr<T>& x)
    {
        aug::smartob<aug_addrob> ob(createaddrob(x));
        verify(aug_insertnbfile
               (nbfiles, ref.get(), nbfilememcb<T>, ob.base()));
    }

    inline void
    removenbfile(mdref ref)
    {
        verify(aug_removenbfile(ref.get()));
    }

    inline void
    foreachnbfile(aug_nbfiles_t nbfiles)
    {
        verify(aug_foreachnbfile(nbfiles));
    }

    inline bool
    emptynbfiles(aug_nbfiles_t nbfiles)
    {
        return verify(aug_emptynbfiles(nbfiles)) ? true : false;
    }

    /**
     * Returns #AUG_FAILINTR if the system call was interrupted.
     */

    inline int
    waitnbevents(aug_nbfiles_t nbfiles, const timeval& timeout)
    {
        return verify(aug_waitnbevents(nbfiles, &timeout));
    }

    inline int
    waitnbevents(aug_nbfiles_t nbfiles)
    {
        return verify(aug_waitnbevents(nbfiles, 0));
    }

    inline void
    shutdownnbfile(mdref ref)
    {
        verify(aug_shutdownnbfile(ref.get()));
    }

    inline void
    setnbeventmask(mdref ref, unsigned short mask)
    {
        verify(aug_setnbeventmask(ref.get(), mask));
    }

    inline unsigned short
    nbeventmask(mdref ref)
    {
        return verify(aug_nbeventmask(ref.get()));
    }

    inline unsigned short
    nbevents(mdref ref)
    {
        return verify(aug_nbevents(ref.get()));
    }
}

#endif // AUGNETPP_NBFILE_HPP
