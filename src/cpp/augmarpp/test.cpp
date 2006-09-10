/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augmarpp.hpp"

#include "augsys.h"

#include <fstream>
#include <string>
#include <strstream>

// SUNWspro8 does not seem to find mktemp when cstdlib is included (using
// stlport4).

#include <stdlib.h>

#define FIELD_(n, v) { n, v, sizeof(v) }

#define STR1_ "abcdefghijklmnopqrstuvwxyz"
#define STR2_ "ABCDEFGHIJKLMNOPQRSTUVWXYZ"

using namespace aug;
using namespace std;

namespace {

    const char STR1[] = STR1_;
    const unsigned STRLEN1(sizeof(STR1_) - 1);

    const char STR2[] = STR2_;
    const unsigned STRLEN2(sizeof(STR2_) - 1);

    const struct aug_field FIELDS[] = {
        FIELD_("familyname", "Aylett"),
        FIELD_("givenname", "Mark"),
        FIELD_("gender", "Male")
    };

    const unsigned FIELDS_SIZE(sizeof(FIELDS) / sizeof(FIELDS[0]));

    class error : public exception {
        char line_[5 + 10 + 1];
    public:
        explicit
        error(unsigned l)
        {
            strstream s(line_, sizeof(line_));
            s << "line " << l << ends;
        }
        const char*
        what() const NOTHROW
        {
            return line_;
        }
    };

    bool
    iequal(const char* lhs, const char* rhs)
    {
        return 0 == aug_strcasecmp(lhs, rhs);
    }

    void
    checkheader(marref ref)
    {
        if (FIELDS_SIZE != aug::fields(ref))
            throw error(__LINE__);

        unsigned i;
        for (i = 0; FIELDS_SIZE > i; ++i) {

            aug::field field(null);
            aug::getfield(ref, field, i);

            if (!iequal(field.name(), FIELDS[i].name_))
                throw error(__LINE__);

            if (field.size() != FIELDS[i].size_)
                throw error(__LINE__);

            if (0 != memcmp(field.value(), FIELDS[i].value_, FIELDS[i].size_))
                throw error(__LINE__);
        }

        for (i = 0; FIELDS_SIZE > i; ++i) {

            unsigned size;
            const char* value = static_cast<
                const char*>(aug::getfield(ref, FIELDS[i].name_, size));

            if (size != FIELDS[i].size_)
                throw error(__LINE__);

            if (0 != memcmp(value, FIELDS[i].value_, FIELDS[i].size_))
                throw error(__LINE__);
        }

        for (i = 0; FIELDS_SIZE > i; ++i) {

            const char* name;
            if (!aug::toname(ref, name, i))
                throw error(__LINE__);

            if (!iequal(name, FIELDS[i].name_))
                throw error(__LINE__);
        }
    }
    void
    headertest(marref ref)
    {
        for (unsigned i(0); FIELDS_SIZE > i; ++i)
            aug::setfield(ref, aug::field(FIELDS[i]));

        checkheader(ref);

        unsigned ord(aug::unsetfield(ref, FIELDS[1].name_).first);
        if (1 != ord)
            throw error(__LINE__);
        try {
            aug::getfield(ref, FIELDS[1].name_);
            throw error(__LINE__);
        } catch (...) {
        }

        aug::removefields(ref);
        if (0 != aug::fields(ref))
            throw error(__LINE__);
    }

    void
    headertest(const char* dst, const char* src)
    {
        headertest(aug::createmar());
        headertest(aug::openmar(dst, AUG_RDWR | AUG_CREAT, 0664));
    }

    void
    contenttest(marref ref)
    {
        aug::setcontent(ref, STR1);

        unsigned size;
        const char* content = static_cast<
            const char*>(aug::content(ref, size));

        if (size != STRLEN1)
            throw error(__LINE__);

        if (0 != memcmp(content, STR1, size))
            throw error(__LINE__);
    }
    void
    contenttest(const char* dst, const char* src)
    {
        contenttest(aug::createmar());
        contenttest(aug::openmar(dst, AUG_RDWR | AUG_CREAT, 0664));
    }

    void
    inserttest(marref ref, const char* src)
    {
        aug::insertmar(ref, src);
        unsigned size;
        const char* content = static_cast<
            const char*>(aug::content(ref, size));

        if (size != STRLEN1)
            throw error(__LINE__);

        if (0 != memcmp(content, STR1, size))
            throw error(__LINE__);
    }
    void
    inserttest(const char* dst, const char* src)
    {
        ofstream strm(src, ios::out | ios::trunc | ios::binary);
        if (!strm)
            throw error(__LINE__);

        strm << STR1 << flush;
        strm.close();

        inserttest(aug::createmar(), src);
        inserttest(aug::openmar(dst, AUG_RDWR | AUG_CREAT, 0664), src);
    }

    void
    extracttest(const char* dst, marref ref)
    {
        aug::setcontent(ref, STR1);
        aug::extractmar(ref, dst);

        ifstream strm(dst, ios::in | ios::binary);

        char buf[STRLEN1];
        strm.read(buf, STRLEN1);

        if (0 != memcmp(buf, STR1, STRLEN1))
            throw error(__LINE__);
    }
    void
    extracttest(const char* dst, const char* src)
    {
        extracttest(dst, aug::createmar());
        extracttest(dst, aug::openmar(src, AUG_RDWR | AUG_CREAT, 0664));
    }

    void
    copytest(marref ref)
    {
        aug::setcontent(ref, STR1);

        aug::smartmar dst(aug::createmar());
        aug::copymar(dst, ref);

        unsigned size;
        const char* content = static_cast<
            const char*>(aug::content(dst, size));

        if (size != STRLEN1)
            throw error(__LINE__);

        if (0 != memcmp(content, STR1, size))
            throw error(__LINE__);
    }
    void
    copytest(const char* dst, const char* src)
    {
        copytest(aug::createmar());
        copytest(aug::openmar(src, AUG_RDWR | AUG_CREAT, 0664));
    }
    void
    opencopytest(const char* dst, const char* src)
    {
        smartmar from(aug::openmar(src, AUG_RDWR | AUG_CREAT, 0664));
        aug::setcontent(from, STR1);

        smartmar to(aug::openmar(dst, AUG_RDWR | AUG_CREAT, 0664));
        aug::copymar(to, from);

        unsigned size;
        const char* content = static_cast<
            const char*>(aug::content(to, size));

        if (size != STRLEN1)
            throw error(__LINE__);

        if (0 != memcmp(content, STR1, size))
            throw error(__LINE__);
    }

    void
    iteratortest(marref ref)
    {
        aug::header header(ref);
        for (unsigned i(0); FIELDS_SIZE > i; ++i)
            header.setfield(aug::field(FIELDS[i]));

        checkheader(ref);

        if ((unsigned)(header.end() - header.begin()) != FIELDS_SIZE)
            throw error(__LINE__);

        for (unsigned j(0); FIELDS_SIZE > j; ++j)
            if (!iequal(header.begin()[j], FIELDS[j].name_))
                throw error(__LINE__);

        aug::header::const_reverse_iterator itR(header.rbegin()),
            endR(header.rend());
        for (; itR != endR; ++itR) {

            unsigned ord(aug::toord(itR));
            if (!iequal(*itR, FIELDS[ord].name_))
                throw error(__LINE__);

            if (0 != memcmp(header.getfield(itR), FIELDS[ord].value_,
                            FIELDS[ord].size_))
                throw error(__LINE__);
        }

        aug::header::const_iterator it(header.find("givenname"));
        if (it == header.end())
            throw error(__LINE__);

        if (0 != memcmp(header.getfield(it), FIELDS[1].value_,
                        FIELDS[1].size_))
            throw error(__LINE__);

        if (header.find("badname") != header.end())
            throw error(__LINE__);
    }
    void
    iteratortest(const char* dst, const char* src)
    {
        iteratortest(aug::createmar());
        iteratortest(aug::openmar(dst, AUG_RDWR | AUG_CREAT, 0664));
    }

    void
    streamtest(marref ref)
    {
        aug::iomarstream strm(ref);
        strm << STR1 << STR2;
        strm << endl;

        strm.seekg(STRLEN1);

        string out;
        getline(strm, out);

        if (out != STR2)
            throw error(__LINE__);
    }
    void
    streamtest(const char* dst, const char* src)
    {
        streamtest(aug::createmar());
        streamtest(aug::openmar(dst, AUG_RDWR | AUG_CREAT, 0664));
    }

    struct test {
        const char* name_;
        void (*test_)(const char*, const char*);
    }
    tests[] = {
        { "header functions", headertest },
        { "content functions", contenttest },
        { "insert function", inserttest },
        { "extract function", extracttest },
        { "copy function", copytest },
        { "opencopy function", opencopytest },
        { "iterator class", iteratortest },
        { "stream class", streamtest }
    };

    bool
    run(const test& t, const char* dst, const char* src)
    {
        bool result(true);
        try {
            cout << "testing " << t.name_ << "... ";
            (*t.test_)(dst, src);
            cout << "ok\n";
        } catch (const aug::errinfo_error& e) {
            aug_perrinfo(e.what());
            result = false;
        } catch (const exception& e) {
            cerr << "error: " << e.what() << endl;
            result = false;
        } catch (...) {
            cerr << "error: unknown exception\n";
            result = false;
        }
        if (!result)
            cout << "fail\n";
        unlink(dst);
        unlink(src);
        return result;
    }
}

int
main(int argc, char* argv[])
{
    static const unsigned TOTAL(sizeof(tests) / sizeof(tests[0]));

    struct aug_errinfo errinfo;
    char dst[] = "dst.XXXXXX";
    char src[] = "src.XXXXXX";

    if (-1 == aug_atexitinit(&errinfo)) {
        cerr << "aug_atexitinit() failed\n";
        return 1;
    }

    if (!mktemp(dst) || !mktemp(src)) {
        cerr << "mktemp() failed\n";
        return 1;
    }

    unsigned failed(0);
    for (unsigned i(0); TOTAL > i; ++i)
        if (!run(tests[i], dst, src))
            ++failed;

    if (failed) {

        cout << failed << " out of " << TOTAL << " tests FAILED\n";
        return 1;
    }

    cout << "all tests PASSED\n";
    return 0;
}
