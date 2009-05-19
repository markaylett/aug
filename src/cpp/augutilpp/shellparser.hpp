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
#ifndef AUGUTILPP_SHELLPARSER_HPP
#define AUGUTILPP_SHELLPARSER_HPP

#include "augutilpp/lexer.hpp"

#include <string>
#include <deque>

namespace aug {

    typedef std::deque<std::pair<std::string, std::string> > shellpairs;

    class shellparser : public mpool_ops {
        lexer lexer_;
        std::string first_;
        shellpairs pairs_;
        bool
        consume(unsigned flags)
        {
            if ((flags & AUG_LEXLABEL))
                first_ = lexertoken(lexer_);
            else if ((flags & AUG_LEXWORD)) {
                if (first_.empty())
                    pairs_.push_back(make_pair(lexertoken(lexer_),
                                               std::string()));
                else {
                    pairs_.push_back(make_pair(first_, lexertoken(lexer_)));
                    first_.clear();
                }
            }
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
        reset(shellpairs& pairs)
        {
            pairs_.swap(pairs);
            pairs_.clear();
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
    resetshell(shellparser& parser, shellpairs& pairs)
    {
        parser.reset(pairs);
    }
}

#endif // AUGUTILPP_SHELLPARSER_HPP
