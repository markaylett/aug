/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/

#include "augutilpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

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
        deque<string> words;
        stringstream ss;
        parser.reset(words);
        copy(words.begin(), words.end(),
             ostream_iterator<string>(ss, "]["));
        string s(ss.str());
        // Insert before erase for empty strings.
        s.insert(0, "[");
        return s.erase(s.size() - 1);
    }

    void
    test(const string& s)
    {
        shellparser parser(getmpool(aug_tlx));
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
        autobasictlx();
        test("x=\"aaa a\" 'b''bb' ccc\\\n xyz");
        test("");
    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
