/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#ifndef AUGUTILPP_SHELLPARSER_HPP
#define AUGUTILPP_SHELLPARSER_HPP

#include "augutilpp/lexer.hpp"

#include <string>
#include <deque>

namespace aug {

    class shellparser : public mpool_ops {
        lexer lexer_;
        std::deque<std::string> words_;
        bool
        consume(unsigned flags)
        {
            if ((flags & (AUG_LEXLABEL | AUG_LEXWORD)))
                words_.push_back(lexertoken(lexer_));
            return (flags & AUG_LEXPHRASE) ? true : false;
        }
    public:
        explicit
        shellparser(mpoolref mpool, bool pairs = false)
            : lexer_(mpool, 0, shellwords, pairs)
        {
        }
        bool
        append(char ch)
        {
            return consume(appendlexer(lexer_, ch));
        }
        bool
        finish()
        {
            return consume(finishlexer(lexer_));
        }
        void
        reset(std::deque<std::string>& words)
        {
            words.swap(words_);
            words_.clear();
        }
    };

    // For consistency with httpparser.

    inline bool
    appendshell(shellparser& parser, char ch)
    {
        return parser.append(ch);
    }

    inline bool
    finishshell(shellparser& parser)
    {
        return parser.finish();
    }

    inline void
    resetshell(shellparser& parser, std::deque<std::string>& words)
    {
        parser.reset(words);
    }
}

#endif // AUGUTILPP_SHELLPARSER_HPP
