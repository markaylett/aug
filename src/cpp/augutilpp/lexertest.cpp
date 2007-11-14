/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/lexer.hpp"

#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>
#include <vector>

using namespace aug;
using namespace std;

namespace {

    typedef logic_error error;

    class shellparser {
        lexer lexer_;
        vector<string> words_;
        bool
        consume(unsigned flags)
        {
            if ((flags & (AUG_LEXLABEL | AUG_LEXWORD)))
                words_.push_back(lexertoken(lexer_));
            return flags & AUG_LEXPHRASE;
        }
    public:
        explicit
        shellparser(bool pairs = false)
            : lexer_(0, shellwords, pairs)
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
        reset(vector<string>& words)
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
    resetshell(shellparser& parser, vector<string>& words)
    {
        parser.reset(words);
    }

    string
    join(shellparser& parser)
    {
        vector<string> words;
        stringstream ss;
        parser.reset(words);
        copy(words.begin(), words.end(),
             ostream_iterator<string>(ss, "]["));
        string s(ss.str());
        return s.erase(s.size() - 1).insert(0, "[");
    }

    void
    test(const string& s)
    {
        shellparser parser;
        string::const_iterator it(s.begin()), end(s.end());
        for (; it != end; ++it)
            if (parser.append(*it))
                cout << join(parser) << endl;
        if (parser.finish())
            cout << join(parser) << endl;
    }
}

int
main(int argc, char* argv[])
{
    try {
        test("x=\"aaa a\" 'b''bb' ccc\\\n xyz");
    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
