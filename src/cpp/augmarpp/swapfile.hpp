/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#ifndef AUGMARPP_SWAPFILE_HPP
#define AUGMARPP_SWAPFILE_HPP

#include "augmarpp/mar.hpp"

#include "augsyspp/types.hpp"

#include "augctxpp/mpool.hpp"

#include "augsys/limits.h" // AUG_PATH_MAX

#include "augctx/string.h" // aug_strlcpy

#include "augext/log.h"

namespace aug {

    template <typename file_policyT>
    class swapfile : public mpool_ops {
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
        ~swapfile() AUG_NOTHROW
        {
            try {
                close(false);
            } AUG_PERRINFOCATCH;
        }
        swapfile(mpoolref mpool, const char* swap, const char* master)
            : mar_(openmar(mpool, swap, AUG_RDWR | AUG_CREAT | AUG_EXCL)),
              done_(false)
        {
            smartmar src(openmar(mpool, master, AUG_RDONLY));
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
