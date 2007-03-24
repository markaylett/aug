/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGNETPP_FILE_HPP
#define AUGNETPP_FILE_HPP

#include "augnetpp/config.hpp"

#include "augutilpp/var.hpp"

#include "augsyspp/exception.hpp"
#include "augsyspp/smartfd.hpp"

#include "augnet/file.h"

#include "augutil/list.h"

#include "augsys/errno.h"
#include "augsys/log.h"

#include <memory> // auto_ptr<>

namespace aug {

    template <bool (*T)(const aug_var&, int, aug_files&)>
    int
    filecb(const aug_var* var, int fd, aug_files* files) AUG_NOTHROW
    {
        try {
            return T(*var, fd, *files) ? 1 : 0;
        } AUG_SETERRINFOCATCH;

        /**
           Do not remove the file unless explicitly asked to.
        */

        return 1;
    }

    template <typename T, bool (T::*U)(int, aug_files&)>
    int
    filememcb(const aug_var* var, int fd, aug_files* files) AUG_NOTHROW
    {
        try {
            return (static_cast<T*>(var->arg_)->*U)(fd, *files) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    template <typename T>
    int
    filememcb(const aug_var* var, int fd, aug_files* files) AUG_NOTHROW
    {
        try {
            return static_cast<T*>(var->arg_)->filecb(fd, *files) ? 1 : 0;
        } AUG_SETERRINFOCATCH;
        return 1;
    }

    class files {
    public:
        typedef aug_files ctype;
    private:

        friend class file;

        aug_files files_;

        files(const files&);

        files&
        operator =(const files&);

    public:
        ~files() AUG_NOTHROW
        {
            if (-1 == aug_destroyfiles(&files_))
                perrinfo("aug_destroyfiles() failed");
        }

        files()
        {
            AUG_INIT(&files_);
        }

        operator aug_files&()
        {
            return files_;
        }

        operator const aug_files&() const
        {
            return files_;
        }

        bool
        empty() const
        {
            return AUG_EMPTY(&files_);
        }
    };

    inline void
    insertfile(aug_files& files, fdref ref, aug_filecb_t cb,
               const aug_var& var)
    {
        verify(aug_insertfile(&files, ref.get(), cb, &var));
    }

    inline void
    insertfile(aug_files& files, fdref ref, aug_filecb_t cb, const null_&)
    {
        verify(aug_insertfile(&files, ref.get(), cb, 0));
    }

    template <typename T>
    void
    insertfile(aug_files& files, fdref ref, T& x)
    {
        aug_var var = { 0, &x };
        verify(aug_insertfile(&files, ref.get(), filememcb<T>, &var));
    }

    template <typename T>
    void
    insertfile(aug_files& files, fdref ref, std::auto_ptr<T>& x)
    {
        aug_var var;
        verify(aug_insertfile(&files, ref.get(), filememcb<T>,
                              &bindvar<deletearg<T> >(var, *x)));
        x.release();
    }

    inline void
    removefile(aug_files& files, fdref ref)
    {
        verify(aug_removefile(&files, ref.get()));
    }

    inline void
    foreachfile(aug_files& files)
    {
        verify(aug_foreachfile(&files));
    }
}

#endif // AUGNETPP_FILE_HPP
