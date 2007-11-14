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
            if (-1 == aug_destroylexer(lexer_))
                perrinfo("aug_destroylexer() failed");
        }

        lexer(unsigned size, const networds_&)
        {
            verify(lexer_ = aug_createnetlexer(size));
        }

        lexer(unsigned size, const shellwords_&, bool pairs)
        {
            verify(lexer_ = aug_createshelllexer(size, pairs ? 1 : 0));
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
