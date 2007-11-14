/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/lexer.hpp"

#include <iostream>
#include <iterator>
#include <sstream>
#include <vector>

using namespace aug;
using namespace std;

namespace {

    class parser {
        lexer lexer_;
        vector<string> words_;
        bool
        consume(unsigned flags)
        {
            if ((flags & AUG_LEXLABEL) || (flags & AUG_LEXWORD))
                words_.push_back(lexertoken(lexer_));
            return flags & AUG_LEXPHRASE;
        }
    public:
        parser()
            : lexer_(0, shellwords)
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

    string
    join(parser& p)
    {
        vector<string> words;
        stringstream ss;
        p.reset(words);
        copy(words.begin(), words.end(),
             ostream_iterator<string>(ss, ":"));
        string s(ss.str());
        return s.substr(0, s.size() - 1);
    }

    void
    test(const string& s)
    {
        parser p;
        string::const_iterator it(s.begin()), end(s.end());
        for (; it != end; ++it)
            if (p.append(*it))
                cout << join(p) << endl;
        if (p.finish())
            cout << join(p) << endl;
    }
}

int
main(int argc, char* argv[])
{
    try {
        test("\"aaa a\" 'b''bb' ccc");
    } catch (const exception& e) {
        cerr << e.what() << endl;
        return 1;
    }
    return 0;
}
