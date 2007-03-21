/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augnetpp/base64.hpp"

#include <iostream>
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
}

int
main(int argc, char* argv[])
{
    try {

        if (filterbase64(DECODED, strlen(DECODED), AUG_ENCODE64) != ENCODED)
            throw error("encodebase64() failed");

        if (filterbase64(ENCODED, strlen(ENCODED), AUG_DECODE64) != DECODED)
            throw error("decodebase64() failed");

        return 0;

    } AUG_PERRINFOCATCH;
    return 1;
}
