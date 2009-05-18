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
#ifndef AUGUTILPP_LEXER_HPP
#define AUGUTILPP_LEXER_HPP

#include "augutilpp/config.hpp"

#include "augctxpp/exception.hpp"
#include "augctxpp/mpool.hpp"

#include "augutil/lexer.h"

namespace aug {

    const struct networds_ { } networds = networds_();
    const struct shellwords_ { } shellwords = shellwords_();

    class lexer : public mpool_ops {

        aug_lexer_t lexer_;

        lexer(const lexer&);

        lexer&
        operator =(const lexer&);

    public:
        ~lexer() AUG_NOTHROW
        {
            if (lexer_)
                aug_destroylexer(lexer_);
        }

        lexer(const null_&) AUG_NOTHROW
           : lexer_(0)
        {
        }

        lexer(mpoolref mpool, unsigned size, const networds_&)
        {
            verify(lexer_ = aug_createnetlexer(mpool.get(), size));
        }

        lexer(mpoolref mpool, unsigned size, const shellwords_&, bool pairs)
        {
            verify(lexer_ = aug_createshelllexer(mpool.get(), size, pairs
                                                 ? AUG_TRUE : AUG_FALSE));
        }

        void
        swap(lexer& rhs) AUG_NOTHROW
        {
            std::swap(lexer_, rhs.lexer_);
        }

        operator aug_lexer_t()
        {
            return lexer_;
        }

        aug_lexer_t
        get()
        {
            return lexer_;
        }
    };

    inline void
    swap(lexer& lhs, lexer& rhs) AUG_NOTHROW
    {
        lhs.swap(rhs);
    }

    inline unsigned
    appendlexer(aug_lexer_t lexer, char ch)
    {
        return aug_appendlexer(lexer, ch);
    }

    inline unsigned
    finishlexer(aug_lexer_t lexer)
    {
        return aug_finishlexer(lexer);
    }

    inline const char*
    lexertoken(aug_lexer_t lexer)
    {
        return aug_lexertoken(lexer);
    }
}

#endif // AUGUTILPP_LEXER_HPP
