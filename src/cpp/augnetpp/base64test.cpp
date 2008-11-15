/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augnetpp.hpp"
#include "augsyspp.hpp"
#include "augctxpp.hpp"

#include <iostream>
#include <sstream>
#include <strstream>
#include <stdexcept>

using namespace aug;
using namespace std;

namespace {

    const char ENCODED[] = "TWFuIGlzIGRpc3Rpbmd1aXNoZWQsIG5vdCBvbmx5IGJ5IGhpc"
        "yByZWFzb24sIGJ1dCBieSB0aGlzIHNpbmd1bGFyIHBhc3Npb24gZnJvbSBvdGhlciBhb"
        "mltYWxzLCB3aGljaCBpcyBhIGx1c3Qgb2YgdGhlIG1pbmQsIHRoYXQgYnkgYSBwZXJzZ"
        "XZlcmFuY2Ugb2YgZGVsaWdodCBpbiB0aGUgY29udGludWVkIGFuZCBpbmRlZmF0aWdhY"
        "mxlIGdlbmVyYXRpb24gb2Yga25vd2xlZGdlLCBleGNlZWRzIHRoZSBzaG9ydCB2ZWhlb"
        "WVuY2Ugb2YgYW55IGNhcm5hbCBwbGVhc3VyZS4=";

    const char DECODED[] = "Man is distinguished, not only by his reason, but"
        " by this singular passion from other animals, which is a lust of the"
        " mind, that by a perseverance of delight in the continued and"
        " indefatigable generation of knowledge, exceeds the short vehemence"
        " of any carnal pleasure.";

    typedef logic_error error;

    struct test {
        aug_result
        cb(const char* buf, size_t len)
        {
            return AUG_SUCCESS;
        }
    };
}

int
main(int argc, char* argv[])
{
    try {
        autobasictlx();

        if (filterbase64(DECODED, strlen(DECODED), AUG_ENCODE64) != ENCODED)
            throw error("encodebase64() failed");

        if (filterbase64(ENCODED, strlen(ENCODED), AUG_DECODE64) != DECODED)
            throw error("decodebase64() failed");

        test x;
        scoped_boxptr_wrapper<simple_boxptr> ob(&x);
        base64 b64(getmpool(aug_tlx), AUG_ENCODE64,
                   base64memcb<test, &test::cb>, ob);
        appendbase64(b64, DECODED, strlen(DECODED));
        finishbase64(b64);

        char buf[21];
        strstream out(buf, 20);
        stringstream in(ENCODED);
        if (!filterbase64(out, in, AUG_DECODE64))
            buf[20] = '\0';
        else
            out << ends;
        if (0 != strcmp(buf, "Man is distinguished"))
            throw error("decodebase64() failed");

        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
