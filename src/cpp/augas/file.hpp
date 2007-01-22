/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGAS_FILE_HPP
#define AUGAS_FILE_HPP

#include "augas.h"
#include "augas/sess.hpp"

#include "augsyspp.hpp"

namespace augas {

    class file_base {
    public:
        typedef augas_file ctype;
    private:

        virtual augas_file&
        do_file() = 0;

        virtual const augas_file&
        do_file() const = 0;

        virtual const sessptr&
        do_sess() const = 0;

        virtual aug::smartfd
        do_sfd() const = 0;

    public:
        virtual
        ~file_base() AUG_NOTHROW;

        augas_file&
        file()
        {
            return do_file();
        }
        const augas_file&
        file() const
        {
            return do_file();
        }
        const sessptr&
        sess() const
        {
            return do_sess();
        }
        aug::smartfd
        sfd() const
        {
            return do_sfd();
        }
        augas_id
        id() const
        {
            return do_file().id_;
        }
        void*
        user() const
        {
            return do_file().user_;
        }
        operator augas_file&()
        {
            return do_file();
        }
        operator const augas_file&() const
        {
            return do_file();
        }
    };

    typedef aug::smartptr<file_base> fileptr;
}

#endif // AUGAS_FILE_HPP
