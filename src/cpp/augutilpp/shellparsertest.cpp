/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augutilpp/shellparser.hpp"

#include <iostream>
#include <iterator>
#include <sstream>
#include <stdexcept>

using namespace aug;
using namespace std;

namespace {

    typedef logic_error error;

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
