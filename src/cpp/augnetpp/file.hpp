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

namespace aug {

    class filecb_base {

        virtual bool
        do_callback(int fd, aug_files& files) = 0;

    public:
        virtual
        ~filecb_base() AUG_NOTHROW
        {
        }

        bool
        callback(int fd, aug_files& files)
        {
            return do_callback(fd, files);
        }

        bool
        operator ()(int fd, aug_files& files)
        {
            return do_callback(fd, files);
        }
    };

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

    namespace detail {

        inline int
        filecb(int id, const struct aug_var* var, aug_files* files)
        {
            try {
                filecb_base* arg = static_cast<filecb_base*>(var->arg_);
                return arg->callback(id, *files) ? 1 : 0;
            } AUG_PERRINFOCATCH;

            /**
               Do not remove the file unless explicitly asked to.
            */

            return 1;
        }
    }

    inline void
    insertfile(aug_files& files, fdref ref, filecb_base& cb)
    {
        aug_var var = { NULL, &cb };
        verify(aug_insertfile(&files, ref.get(), detail::filecb, &var));
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
