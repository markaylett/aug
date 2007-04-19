/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_NBFILE_HPP
#define AUGNETPP_NBFILE_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/nbfile.h"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"

#include <memory> // auto_ptr<>

namespace aug {

    template <bool (*T)(const aug_var&, int, unsigned short)>
    int
    nbfilecb(const aug_var* var, int fd, unsigned short events) AUG_NOTHROW
    {
        try {
            return T(*var, fd, events) ? 1 : 0;
        } AUG_SETERRINFOCATCH;

        /**
           Do not remove the file unless explicitly asked to.
        */

        return 1;
    }

    template <typename T, bool (T::*U)(const aug_var&, int, unsigned short)>
    int
    nbfilememcb(const aug_var* var, int fd, unsigned short events) AUG_NOTHROW
    {
        try {
            return (static_cast<T*>(var->arg_)->*U)(fd, events) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    template <typename T>
    int
    nbfilememcb(const aug_var* var, int fd, unsigned short events) AUG_NOTHROW
    {
        try {
            return static_cast<T*>(var->arg_)->nbfilecb(fd, events) ? 1 : 0;
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
                perrinfo("aug_destroynbfiles() failed");
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
    insertnbfile(aug_nbfiles_t nbfiles, fdref ref, aug_nbfilecb_t cb,
               const aug_var& var)
    {
        verify(aug_insertnbfile(nbfiles, ref.get(), cb, &var));
    }

    inline void
    insertnbfile(aug_nbfiles_t nbfiles, fdref ref, aug_nbfilecb_t cb,
               const null_&)
    {
        verify(aug_insertnbfile(nbfiles, ref.get(), cb, 0));
    }

    template <typename T>
    void
    insertnbfile(aug_nbfiles_t nbfiles, fdref ref, T& x)
    {
        aug_var var = { 0, &x };
        verify(aug_insertnbfile(nbfiles, ref.get(), nbfilememcb<T>, &var));
    }

    template <typename T>
    void
    insertnbfile(aug_nbfiles_t nbfiles, fdref ref, std::auto_ptr<T>& x)
    {
        aug_var var;
        verify(aug_insertnbfile(nbfiles, ref.get(), nbfilememcb<T>,
                                &bindvar<deletearg<T> >(var, *x)));
        x.release();
    }

    inline void
    removenbfile(fdref ref)
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
       Returns #AUG_RETINTR if the system call was interrupted.
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
    shutdownnbfile(fdref ref)
    {
        verify(aug_shutdownnbfile(ref.get()));
    }

    inline void
    setnbeventmask(fdref ref, unsigned short mask)
    {
        verify(aug_setnbeventmask(ref.get(), mask));
    }

    inline unsigned short
    nbeventmask(fdref ref)
    {
        return verify(aug_nbeventmask(ref.get()));
    }

    inline unsigned short
    nbevents(fdref ref)
    {
        return verify(aug_nbevents(ref.get()));
    }
}

#endif // AUGNETPP_NBFILE_HPP
