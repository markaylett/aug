/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
/**
   \file swapfile.hpp
   \brief TODO
 */

#ifndef AUGMARPP_SWAPFILE_HPP
#define AUGMARPP_SWAPFILE_HPP

#include "augmarpp/mar.hpp"

#include "augsys/limits.h" // AUG_PATH_MAX
#include "augsys/log.h"
#include "augsys/string.h" // aug_strlcpy

namespace aug {

    template <typename file_policyT>
    class swapfile {
    public:
        typedef file_policyT file_policy_type;

    private:
        char swap_[AUG_PATH_MAX + 1];
        char master_[AUG_PATH_MAX + 1];
        smartmar mar_;
        bool done_;

        swapfile(const swapfile& rhs);

        swapfile&
        operator =(const swapfile& rhs);

    public:
        ~swapfile() NOTHROW
        {
            try {
                close(false);
            } AUG_PERRINFOCATCH;
        }
        swapfile(const char* swap, const char* master)
            : mar_(openmar(swap, AUG_RDWR | AUG_CREAT | AUG_EXCL)),
              done_(false)
        {
            smartmar src(openmar(master, AUG_RDONLY));
            copymar(mar_, src);

            aug_strlcpy(swap_, swap, sizeof(swap_));
            aug_strlcpy(master_, master, sizeof(master_));
        }
        void
        close(bool commit = true)
        {
            if (done_)
                return;

            done_ = true;

            try {
                mar_.release();
                if (commit)
                    file_policy_type::rename(swap_, master_);
                else
                    file_policy_type::unlink(swap_);

            } catch (const std::exception& e) {
                file_policy_type::unlink(swap_);
                throw;
            }
        }
        void
        commit()
        {
            close(true);
        }
        void
        rollback()
        {
            close(false);
        }
    };
}

#endif // AUGMARPP_SWAPFILE_HPP
