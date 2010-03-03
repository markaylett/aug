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
#include "augctx/defs.h"

AUG_RCSID("$Id$");

/**
 * @page mar
 *
 * Tool for manipulating Meta ARchive files.
 */

#include "mar/getopt.h"
#include "mar/utility.h"

#include "augsys/debug.h"

#include "augctx/base.h"
#include "augctx/errinfo.h"
#include "augctx/errno.h"
#include "augctx/utility.h" /* aug_perrinfo() */

#include "augext/blob.h"

#include <stdlib.h>
#include <string.h>

#define CREATEOPT_ 0x01
#define FORCEOPT_ 0x02
#define READOPT_ 0x04
#define STDINOPT_ 0x08
#define WRITEOPT_ 0x10

#define OPTIONS_ "cd:fg:hi:lnop:tx:z"

#define CLEARTEXT_ "clear all fields"
#define COMPACTTEXT_ "compact archive"
#define DELTEXT_ "delete field by name"
#define EXTRACTTEXT_ "extract content data into file"
#define FORCETEXT_ "no prompt for confirmation"
#define GETTEXT_ "get field value by name"
#define HELPTEXT_ "display this help, then exit"
#define INSERTTEXT_ "insert content data from file"
#define LISTTEXT_ "list all fields"
#define NAMESTEXT_ "list all field names"
#define PUTTEXT_ "put field(s) from source"
#define SIZETEXT_ "print size of content data"
#define ZEROTEXT_ "zero truncate content data"

#define FORCE_ (FORCEOPT_ & options_)

static unsigned options_ = 0;

static aug_result
fileset_(aug_mar* mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {

        if (aug_streamset_(mar, stdin) < 0)
            return -1;

    } else {

        FILE* stream;

        if (!(stream = fopen(filename, "r"))) {
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }

        if (aug_streamset_(mar, stream) < 0) {
            fclose(stream);
            return -1;
        }

        if (0 != fclose(stream)) {
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }
    }

    return 0;
}

static aug_result
clear_(aug_mar* mar)
{
    if (!FORCE_ && !aug_confirm_(CLEARTEXT_))
        return 0;

    return aug_clearfields(mar);
}

static aug_result
del_(aug_mar* mar, const char* name)
{
    if (!FORCE_ && !aug_confirm_(DELTEXT_))
        return 0;

    return aug_delfieldp(mar, name);
}

static aug_result
extract_(aug_mar* mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {

        aug_blob* blob = aug_cast(mar, aug_blobid);
        unsigned size;
        const void* body;

        if (!(body = aug_getblobdata(blob, &size))) {
            aug_release(blob);
            return -1;
        }

        if (size != fwrite(body, 1, size, stdout)) {
            aug_release(blob);
            aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
            return -1;
        }

        aug_release(blob);

    } else {

        if (aug_extractmar(mar, filename) < 0)
            return -1;
    }
    return 0;
}

static aug_result
get_(aug_mar* mar, const char* name)
{
    const void* value;
    aug_rint ret = aug_getfieldp(mar, name, &value);
    if (ret < 0 || aug_writevalue_(stdout, value, ret) < 0)
        return -1;

    if (EOF == fflush(stdout)) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static void
help_(void)
{
    /* Help text is split to avoid compiler warnings regarding length of
       string constant. */

    static const char OPTIONHELP_[] =
        "  -c           " CLEARTEXT_ "\n"
        "  -d name      " DELTEXT_ "\n"
        "  -f           " FORCETEXT_ "\n"
        "  -g name      " GETTEXT_ "\n"
        "  -h           " HELPTEXT_ "\n"
        "  -i filename  " INSERTTEXT_ "\n"
        "  -l           " NAMESTEXT_ "\n"
        "  -n           " SIZETEXT_ "\n"
        "  -o           " COMPACTTEXT_ "\n"
        "  -p source    " PUTTEXT_ "\n"
        "  -t           " LISTTEXT_ "\n"
        "  -x filename  " EXTRACTTEXT_ "\n"
        "  -z           " ZEROTEXT_ "\n";

    printf("Message Archive Utility\n"
           "\nusage:\n"
           "  mar option... archivename\n"
           "\noptions:\n%s"
           "\nreport bugs to <mark.aylett@gmail.com>\n", OPTIONHELP_);
}

static aug_result
insert_(aug_mar* mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {
        if (aug_insertstream_(mar, stdin) < 0)
            return -1;
    } else {
        if (aug_insertmar(mar, filename) < 0)
            return -1;
    }
    return 0;
}

static aug_result
list_(aug_mar* mar)
{
    int i;
    for (i = 0; ; ++i) {

        struct aug_field field;
        if (aug_getfield(mar, i, &field) < 0)
            return -1;

        if (!field.value_)
            break;

        printf("%s=", field.name_);

        if (aug_writevalue_(stdout, field.value_, field.size_) < 0)
            return -1;
    }

    if (EOF == fflush(stdout)) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static aug_result
names_(aug_mar* mar)
{
    int i;
    for (i = 0; ; ++i) {

        const char* name;
        if (aug_fieldntop(mar, i, &name) < 0) {

            if (AUG_EXNONE == aug_getexcept(aug_tlx))
                break;

            return -1;
        }
        printf("%s\n", name);
    }
    if (EOF == fflush(stdout)) {
        aug_setposixerror(aug_tlx, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

static aug_result
put_(aug_mar* mar, char* src)
{
    struct aug_field field;

    if (aug_atofield_(&field, src) < 0) {

        /* Not a pair, so assume file name. */

        return fileset_(mar, src);
    }

    return aug_putfield(mar, &field);
}

static void
size_(aug_mar* mar)
{
    aug_blob* blob = aug_cast(mar, aug_blobid);
    size_t size = aug_getblobsize(blob);
    aug_release(blob);

    printf("%u\n", (unsigned)size);
}

static aug_result
zero_(aug_mar* mar)
{
    if (!FORCE_ && !aug_confirm_(ZEROTEXT_))
        return 0;

    return aug_truncatemar(mar, 0);
}

static void
exit_(void)
{
    AUG_DUMPLEAKS();
}

static aug_result
run_(int argc, char* argv[], const char* archivename)
{
    aug_mpool* mpool;
    int flags = 0;
    mode_t mode = 0;

    aug_mar* mar;
    int ch;

    switch (options_ & (READOPT_ | WRITEOPT_)) {
    case READOPT_:
        flags = AUG_RDONLY;
        break;
    case WRITEOPT_:
        flags = AUG_WRONLY;
        break;
    case READOPT_ | WRITEOPT_:
        flags = AUG_RDWR;
        break;
    }

    if (options_ & CREATEOPT_) {
        flags |= AUG_CREAT;
        mode = 0666;
    }

    mpool = aug_getmpool(aug_tlx);
    mar = aug_openmar(mpool, archivename, flags, mode);
    aug_release(mpool);

    if (!mar) {
        aug_perrinfo(aug_tlx, "aug_openmar() failed", NULL);
        return -1;
    }

    while (-1 != (ch = aug_getopt(argc, argv, OPTIONS_)))
        switch (ch) {
        case 'c':
            if (clear_(mar) < 0) {
                aug_perrinfo(aug_tlx, "failed to " CLEARTEXT_, NULL);
                goto fail;
            }
            break;
        case 'd':
            if (del_(mar, aug_optarg) < 0) {
                aug_perrinfo(aug_tlx, "failed to " DELTEXT_, NULL);
                goto fail;
            }
            break;
        case 'f':
            break;
        case 'g':
            if (get_(mar, aug_optarg) < 0) {
                aug_perrinfo(aug_tlx, "failed to " GETTEXT_, NULL);
                goto fail;
            }
            break;
        case 'i':
            if (insert_(mar, aug_optarg) < 0) {
                aug_perrinfo(aug_tlx, "failed to " INSERTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'l':
            if (names_(mar) < 0) {
                aug_perrinfo(aug_tlx, "failed to " NAMESTEXT_, NULL);
                goto fail;
            }
            break;
        case 'n':
            size_(mar);
            break;
        case 'o':
            if (aug_compactmar(mar) < 0) {
                aug_perrinfo(aug_tlx, "failed to " COMPACTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'p':
            if (put_(mar, aug_optarg) < 0) {
                aug_perrinfo(aug_tlx, "failed to " PUTTEXT_, NULL);
                goto fail;
            }
            break;
        case 't':
            if (list_(mar) < 0) {
                aug_perrinfo(aug_tlx, "failed to " LISTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'x':
            if (extract_(mar, aug_optarg) < 0) {
                aug_perrinfo(aug_tlx, "failed to " EXTRACTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'z':
            if (zero_(mar) < 0) {
                aug_perrinfo(aug_tlx, "failed to " ZEROTEXT_, NULL);
                goto fail;
            }
            break;
        case 'h':
        case '?':
        default:
            fprintf(stderr, "unexpected option [-%c]\n", aug_optopt);
            goto fail;
        }

    aug_release(mar);
    return 0;

 fail:
    aug_release(mar);
    return -1;
}

int
main(int argc, char* argv[])
{
    char ch;
    const char* archivename = NULL;
    aug_opterr = 0;

    if (!aug_autotlx())
        return 1;

    AUG_INITLEAKDUMP();

    if (atexit(exit_) < 0) {
        aug_perrinfo(aug_tlx, "atexit() failed", NULL);
        goto fail;
    }

    while ((ch = aug_getopt(argc, argv, OPTIONS_)) < 0)
        switch (ch) {
        case 'i':
        case 'p':
            if (0 == strcmp(aug_optarg, "-")) {

                if (options_ & STDINOPT_) {
                    fprintf(stderr, "multiple use of stdin");
                    goto info;
                }

                options_ |= STDINOPT_;
            }
            options_ |= (CREATEOPT_ | WRITEOPT_);
            break;
        case 'f':
            options_ |= FORCEOPT_;
            break;
        case 'h':
            help_();
            return 0;
        case 'g':
        case 'l':
        case 'n':
        case 't':
        case 'x':
            options_ |= READOPT_;
            break;
        case 'c':
        case 'd':
        case 'o':
        case 'z':
            options_ |= WRITEOPT_;
            break;
        case '?':
        default:
            fprintf(stderr, "unknown option [-%c]", aug_optopt);
            goto info;
        }

    if (aug_optind < (argc - 1)) {
        fprintf(stderr, "too many arguments");
        goto info;
    }

    if (aug_optind == argc) {
        fprintf(stderr, "archive not specified");
        goto info;
    }

    archivename = argv[aug_optind];
    aug_optind = 0;

    return run_(argc, argv, archivename) < 0 ? 1 : 0;

 info:
    fprintf(stderr, ": try 'mar -h' for more information\n");
    return 1;
}
