/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
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

#include <stdlib.h>
#include <string.h>

#define CREATEOPT_ 0x01
#define FORCEOPT_ 0x02
#define READOPT_ 0x04
#define STDINOPT_ 0x08
#define WRITEOPT_ 0x10

#define OPTIONS_ "cfg:hi:lnrs:tu:x:z"

#define COMPACTTEXT_ "compact archive"
#define FORCETEXT_ "no prompt for confirmation"
#define GETTEXT_ "print field value by name"
#define HELPTEXT_ "display this help, then exit"
#define INSERTTEXT_ "insert content data from file"
#define NAMESTEXT_ "list all field names"
#define SIZETEXT_ "print size of content data"
#define LISTTEXT_ "list all fields"
#define REMOVETEXT_ "remove all fields"
#define SETTEXT_ "set field(s) from source"
#define UNSETTEXT_ "unset field by name"
#define EXTRACTTEXT_ "extract content data into file"
#define ZEROTEXT_ "zero truncate content data"

#define FORCE_ (FORCEOPT_ & options_)

static unsigned options_ = 0;

static aug_result
fileset_(aug_mar_t mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {

        aug_verify(aug_streamset_(mar, stdin));

    } else {

        FILE* stream;
        aug_result result;

        if (!(stream = fopen(filename, "r")))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

        if (AUG_ISFAIL(result = aug_streamset_(mar, stream))) {
            fclose(stream);
            return AUG_FAILERROR;
        }

        if (0 != fclose(stream))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    }

    return AUG_SUCCESS;
}

static aug_result
extract_(aug_mar_t mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {

        const void* body;
        unsigned size;

        if (!(body = aug_getcontent(mar, &size)))
            return AUG_FAILERROR;

        if (size != fwrite(body, 1, size, stdout))
            return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    } else {

        aug_verify(aug_extractmar(mar, filename));
    }
    return AUG_SUCCESS;
}

static void
help_(void)
{
    /* Help text is split to avoid compiler warnings regarding length of
       string constant. */

    static const char OPTIONHELP_[] =
        "  -c           " COMPACTTEXT_ "\n"
        "  -f           " FORCETEXT_ "\n"
        "  -g name      " GETTEXT_ "\n"
        "  -h           " HELPTEXT_ "\n"
        "  -i filename  " INSERTTEXT_ "\n"
        "  -l           " NAMESTEXT_ "\n"
        "  -n           " SIZETEXT_ "\n"
        "  -r           " REMOVETEXT_ "\n"
        "  -s source    " SETTEXT_ "\n"
        "  -t           " LISTTEXT_ "\n"
        "  -u name      " UNSETTEXT_ "\n"
        "  -x filename  " EXTRACTTEXT_ "\n"
        "  -z           " ZEROTEXT_ "\n";

    printf("Message Archive Utility\n"
           "\nusage:\n"
           "  mar option... archivename\n"
           "\noptions:\n%s"
           "\nreport bugs to <mark@emantic.co.uk>\n", OPTIONHELP_);
}


static aug_result
insert_(aug_mar_t mar, const char* filename)
{
    aug_result result;

    if (0 == strcmp(filename, "-")) {

        result = aug_insertstream_(mar, stdin);

    } else {

        result = aug_insertmar(mar, filename);
    }

    return AUG_ISFAIL(result) ? result : AUG_SUCCESS;
}

static aug_result
names_(aug_mar_t mar)
{
    int i;
    for (i = 0; ; ++i) {

        const char* name;
        aug_verify(aug_ordtoname(mar, &name, i));

        if (NULL == name)
            break;

        printf("%s\n", name);
    }
    if (EOF == fflush(stdout))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

static void
size_(aug_mar_t mar)
{
    unsigned size = aug_getcontentsize(mar);
    printf("%u\n", size);
}

static aug_result
list_(aug_mar_t mar)
{
    int i;
    for (i = 0; ; ++i) {

        struct aug_field field;

        aug_verify(aug_getfield(mar, &field, i));

        if (!field.value_)
            break;

        printf("%s=", field.name_);

        aug_verify(aug_writevalue_(stdout, field.value_, field.size_));
    }

    if (EOF == fflush(stdout))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

static aug_result
get_(aug_mar_t mar, const char* name)
{
    const void* value;
    unsigned size;

    if (!(value = aug_valuebyname(mar, name, &size)))
        return AUG_FAILERROR;

    aug_verify(aug_writevalue_(stdout, value, size));

    if (EOF == fflush(stdout))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}

static aug_result
remove_(aug_mar_t mar)
{
    if (!FORCE_ && !aug_confirm_(REMOVETEXT_))
        return AUG_SUCCESS;

    return aug_removefields(mar);
}

static aug_result
set_(aug_mar_t mar, char* src)
{
    struct aug_field field;

    if (AUG_ISFAIL(aug_atofield_(&field, src))) {

        /* Not a pair, so assume file name. */

        return fileset_(mar, src);
    }

    return aug_setfield(mar, &field, NULL);
}

static aug_result
unset_(aug_mar_t mar, const char* name)
{
    if (!FORCE_ && !aug_confirm_(UNSETTEXT_))
        return AUG_SUCCESS;

    return aug_unsetbyname(mar, name, NULL);
}

static aug_result
zero_(aug_mar_t mar)
{
    if (!FORCE_ && !aug_confirm_(ZEROTEXT_))
        return AUG_SUCCESS;

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

    aug_mar_t mar;
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
        return AUG_FAILERROR;
    }

    while (-1 != (ch = aug_getopt(argc, argv, OPTIONS_)))
        switch (ch) {
        case 'c':
            if (AUG_ISFAIL(aug_compactmar(mar))) {
                aug_perrinfo(aug_tlx, "failed to " COMPACTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'f':
            break;
        case 'g':
            if (AUG_ISFAIL(get_(mar, aug_optarg))) {
                aug_perrinfo(aug_tlx, "failed to " GETTEXT_, NULL);
                goto fail;
            }
            break;
        case 'i':
            if (AUG_ISFAIL(insert_(mar, aug_optarg))) {
                aug_perrinfo(aug_tlx, "failed to " INSERTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'l':
            if (AUG_ISFAIL(names_(mar))) {
                aug_perrinfo(aug_tlx, "failed to " NAMESTEXT_, NULL);
                goto fail;
            }
            break;
        case 'n':
            size_(mar);
            break;
        case 'r':
            if (AUG_ISFAIL(remove_(mar))) {
                aug_perrinfo(aug_tlx, "failed to " REMOVETEXT_, NULL);
                goto fail;
            }
            break;
        case 's':
            if (AUG_ISFAIL(set_(mar, aug_optarg))) {
                aug_perrinfo(aug_tlx, "failed to " SETTEXT_, NULL);
                goto fail;
            }
            break;
        case 't':
            if (AUG_ISFAIL(list_(mar))) {
                aug_perrinfo(aug_tlx, "failed to " LISTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'u':
            if (AUG_ISFAIL(unset_(mar, aug_optarg))) {
                aug_perrinfo(aug_tlx, "failed to " UNSETTEXT_, NULL);
                goto fail;
            }
            break;
        case 'x':
            if (AUG_ISFAIL(extract_(mar, aug_optarg))) {
                aug_perrinfo(aug_tlx, "failed to " EXTRACTTEXT_, NULL);
                goto fail;
            }
            break;
        case 'z':
            if (AUG_ISFAIL(zero_(mar))) {
                aug_perrinfo(aug_tlx, "failed to " ZEROTEXT_, NULL);
                goto fail;
            }
            break;
        case 'h':
        case '?':
        default:
            fprintf(stderr, "unexpected option '-%c'\n", aug_optopt);
            goto fail;
        }

    aug_releasemar(mar);
    return AUG_SUCCESS;

 fail:
    aug_releasemar(mar);
    return AUG_FAILERROR;
}

int
main(int argc, char* argv[])
{
    char ch;
    const char* archivename = NULL;
    aug_opterr = 0;

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    AUG_INITLEAKDUMP();

    if (-1 == atexit(exit_)) {
        aug_perrinfo(aug_tlx, "atexit() failed", NULL);
        goto fail;
    }

    while (-1 != (ch = aug_getopt(argc, argv, OPTIONS_)))
        switch (ch) {
        case 'i':
        case 's':
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
        case 'k':
        case 'l':
        case 'n':
        case 't':
        case 'x':
            options_ |= READOPT_;
            break;
        case 'c':
        case 'r':
        case 'u':
        case 'z':
            options_ |= WRITEOPT_;
            break;
        case '?':
        default:
            fprintf(stderr, "unknown option `-%c'", aug_optopt);
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

    if (AUG_ISFAIL(run_(argc, argv, archivename)))
        goto fail;

    return 0;

 info:
    fprintf(stderr, ": try `mar -h' for more information\n");
 fail:
    return 1;
}
