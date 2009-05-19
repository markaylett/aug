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

    struct tostring : unary_function<pair<string, string>, string> {
        result_type
        operator ()(const argument_type& p)
        {
            string s(p.first);
            if (!p.second.empty()) {
                s += '=';
                s += p.second;
            }
            return s;
        }
    };

    string
    join(shellparser& parser)
    {
        shellpairs pairs;
        parser.reset(pairs);

        vector<string> words;
        transform(pairs.begin(), pairs.end(), back_inserter(words),
                  tostring());

        stringstream ss;
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
        autotlx();
        test("x=\"aaa a\" 'b''bb' ccc\\\n xyz");
        test("");
    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
        return 1;
    }
    return 0;
}
