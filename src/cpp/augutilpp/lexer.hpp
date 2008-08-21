/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_LEXER_HPP
#define AUGUTILPP_LEXER_HPP

#include "augutilpp/config.hpp"

#include "augsyspp/utility.hpp"

#include "augutil/lexer.h"

namespace aug {

    const struct networds_ { } networds = networds_();
    const struct shellwords_ { } shellwords = shellwords_();

    class lexer {

        aug_lexer_t lexer_;

        lexer(const lexer&);

        lexer&
        operator =(const lexer&);

    public:
        ~lexer() AUG_NOTHROW
        {
            if (lexer_ && -1 == aug_destroylexer(lexer_))
                perrinfo(aug_tlx, "aug_destroylexer() failed");
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
            verify(lexer_ = aug_createshelllexer(mpool.get(), size,
                                                 pairs ? 1 : 0));
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
