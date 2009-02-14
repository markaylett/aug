/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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
