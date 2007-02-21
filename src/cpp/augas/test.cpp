/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augaspp.hpp"

#include <iostream>

using namespace augas;
using namespace std;

namespace {

    const char DECODED[] = "\"hello, world!\"";
    const char ENCODED[] = "%22hello%2C+world!%22";
    const char LTRIMMED[] = "hello, world!  \r\n";
    const char RTRIMMED[] = "\t  hello, world!";
    const char TRIMMED[] = "hello, world!";
    const char UNTRIMMED[] = "\t  hello, world!  \r\n";

    void
    urlencodetest()
    {
        if (urlencode(DECODED, DECODED + strlen(DECODED)) != ENCODED)
            throw error("urlencode() failed");
    }

    void
    urldecodetest()
    {
        if (urldecode(ENCODED, ENCODED + strlen(ENCODED)) != DECODED)
            throw error("urldecode() failed");
    }

    void
    ltrimtest()
    {
        string s(UNTRIMMED);
        if (ltrim(s) != LTRIMMED)
            throw error("ltrim() failed");

        if (ltrimcopy(UNTRIMMED) != LTRIMMED)
            throw error("ltrimcopy() failed");
    }

    void
    rtrimtest()
    {
        string s(UNTRIMMED);
        if (rtrim(s) != RTRIMMED)
            throw error("rtrim() failed");

        if (rtrimcopy(UNTRIMMED) != RTRIMMED)
            throw error("rtrimcopy() failed");
    }

    void
    trimtest()
    {
        string s(UNTRIMMED);
        if (trim(s) != TRIMMED)
            throw error("trim() failed");

        if (trimcopy(UNTRIMMED) != TRIMMED)
            throw error("trimcopy() failed");
    }

    void
    copyiftest()
    {
        string in("one:two"), out;
        string::iterator it(in.begin());

        it = copyif(it, in.end(), back_inserter(out),
                    bind2nd(not_equal_to<char>(), ':'));
        if (out != "one" || it == in.end() || *it != ':')
            throw error("copyif() failed");

        it = copyif(++it, in.end(), back_inserter(out),
                    bind2nd(not_equal_to<char>(), ':'));
        if (out != "onetwo" || it != in.end())
            throw error("copyif() failed");
    }
}

int
main(int argc, char* argv[])
{
    try {
        urlencodetest();
        urldecodetest();
        ltrimtest();
        rtrimtest();
        trimtest();
        copyiftest();
        return 0;
    } catch (const exception& e) {
        cerr << "error: " << e.what() << endl;
    }
    return 1;
}
