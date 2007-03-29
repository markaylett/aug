/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

static const char rcsid[] = "$Id$";

#include "mar/getopt.h"
#include "mar/utility.h"

#include "augsys/debug.h"
#include "augsys/errinfo.h"
#include "augsys/errno.h"

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

static int
fileset_(aug_mar_t mar, const char* filename)
{
    FILE* stream;

    if (0 == strcmp(filename, "-")) {

        if (-1 == aug_streamset_(mar, stdin))
            return -1;

    } else {

        if (!(stream = fopen(filename, "r"))) {
            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }

        if (-1 == aug_streamset_(mar, stream))
            goto fail;

        if (0 != fclose(stream)) {
            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }
    }
    return 0;

 fail:
    fclose(stream);
    return -1;
}

static int
extract_(aug_mar_t mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {

        const void* body;
        unsigned size;

        if (!(body = aug_content(mar, &size)))
            return -1;

        if (size != fwrite(body, 1, size, stdout)) {
            aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
            return -1;
        }

    } else {

        if (-1 == aug_extractmar(mar, filename))
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


static int
insert_(aug_mar_t mar, const char* filename)
{
    if (0 == strcmp(filename, "-")) {

        if (-1 == aug_insertstream_(mar, stdin))
            return -1;

    } else {

        if (-1 == aug_insertmar(mar, filename))
            return -1;
    }
    return 0;
}

static int
names_(aug_mar_t mar)
{
    int i;
    for (i = 0; ; ++i) {

        const char* name;
        if (-1 == aug_ordtoname(mar, &name, i))
            return -1;

        if (NULL == name)
            break;

        printf("%s\n", name);
    }
    if (EOF == fflush(stdout)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static int
size_(aug_mar_t mar)
{
    unsigned size;
    if (-1 == aug_contentsize(mar, &size))
        return -1;

    printf("%d\n", (int)size);
    return 0;
}

static int
list_(aug_mar_t mar)
{
    int i;
    for (i = 0; ; ++i) {

        struct aug_field field;
        if (-1 == aug_getfield(mar, &field, i))
            return -1;

        if (!field.value_)
            break;

        printf("%s=", field.name_);
        if (-1 == aug_writevalue_(stdout, field.value_, field.size_))
            return -1;
    }

    if (EOF == fflush(stdout)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static int
get_(aug_mar_t mar, const char* name)
{
    const void* value;
    unsigned size;

    if (!(value = aug_valuebyname(mar, name, &size)))
        return -1;

    if (-1 == aug_writevalue_(stdout, value, size))
        return -1;

    if (EOF == fflush(stdout)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}

static int
remove_(aug_mar_t mar)
{
    if (!FORCE_ && !aug_confirm_(REMOVETEXT_))
        return 0;

    return aug_removefields(mar);
}

static int
set_(aug_mar_t mar, char* src)
{
    struct aug_field field;

    if (-1 == aug_atofield_(&field, src))
        return fileset_(mar, src);

    return aug_setfield(mar, &field, NULL);
}

static int
unset_(aug_mar_t mar, const char* name)
{
    if (!FORCE_ && !aug_confirm_(UNSETTEXT_))
        return 0;

    return aug_unsetbyname(mar, name, NULL);
}

static int
zero_(aug_mar_t mar)
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

static int
run_(int argc, char* argv[], const char* archivename)
{
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

    if (!(mar = aug_openmar(archivename, flags, mode))) {
        aug_perrinfo(NULL, "aug_openmar() failed");
        return -1;
    }

    while (-1 != (ch = aug_getopt(argc, argv, OPTIONS_)))
        switch (ch) {
        case 'c':
            if (-1 == aug_compactmar(mar)) {
                aug_perrinfo(NULL, "failed to " COMPACTTEXT_);
                goto fail;
            }
            break;
        case 'f':
            break;
        case 'g':
            if (-1 == get_(mar, aug_optarg)) {
                aug_perrinfo(NULL, "failed to " GETTEXT_);
                goto fail;
            }
            break;
        case 'i':
            if (-1 == insert_(mar, aug_optarg)) {
                aug_perrinfo(NULL, "failed to " INSERTTEXT_);
                goto fail;
            }
            break;
        case 'l':
            if (-1 == names_(mar)) {
                aug_perrinfo(NULL, "failed to " NAMESTEXT_);
                goto fail;
            }
            break;
        case 'n':
            if (-1 == size_(mar)) {
                aug_perrinfo(NULL, "failed to " SIZETEXT_);
                goto fail;
            }
            break;
        case 'r':
            if (-1 == remove_(mar)) {
                aug_perrinfo(NULL, "failed to " REMOVETEXT_);
                goto fail;
            }
            break;
        case 's':
            if (-1 == set_(mar, aug_optarg)) {
                aug_perrinfo(NULL, "failed to " SETTEXT_);
                goto fail;
            }
            break;
        case 't':
            if (-1 == list_(mar)) {
                aug_perrinfo(NULL, "failed to " LISTTEXT_);
                goto fail;
            }
            break;
        case 'u':
            if (-1 == unset_(mar, aug_optarg)) {
                aug_perrinfo(NULL, "failed to " UNSETTEXT_);
                goto fail;
            }
            break;
        case 'x':
            if (-1 == extract_(mar, aug_optarg)) {
                aug_perrinfo(NULL, "failed to " EXTRACTTEXT_);
                goto fail;
            }
            break;
        case 'z':
            if (-1 == zero_(mar)) {
                aug_perrinfo(NULL, "failed to " ZEROTEXT_);
                goto fail;
            }
            break;
        case 'h':
        case '?':
        default:
            fprintf(stderr, "unexpected option '-%c'\n", aug_optopt);
            goto fail;
        }

    return aug_releasemar(mar);

 fail:
    aug_releasemar(mar);
    return -1;
}

int
main(int argc, char* argv[])
{
    char ch;
    const char* archivename = NULL;
    aug_opterr = 0;

    AUG_INITLEAKDUMP();

    if (-1 == atexit(exit_)) {
        aug_perrinfo(NULL, "atexit() failed");
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

    if (-1 == run_(argc, argv, archivename))
        goto fail;

    return 0;

 info:
    fprintf(stderr, ": try `mar -h' for more information\n");
 fail:
    return 1;
}
