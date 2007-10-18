/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "mar/utility.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augsys/errinfo.h"

#include <errno.h>
#include <string.h>

static const char NL_ = '\n';

AUG_EXTERNC int
aug_atofield_(struct aug_field* field, char* src)
{
    /* Split field in the form of "name=value". */

    char* value = strchr(src, '=');
    size_t size;

    if (NULL == value) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("empty value part"));
        return -1;
    }

    size = value - src;
    *value = '\0';

    field->name_ = src;
    field->value_ = ++value;
    field->size_ = (unsigned)strlen(value);
    return 0;
}

AUG_EXTERNC int
aug_confirm_(const char* prompt)
{
    char buf[3];

    printf("%s? (y or [n]): ", prompt);
    fflush(stdout);

    switch (aug_readline_(buf, sizeof(buf), stdin)) {
    case 1:
        break;
    default:
        return 0;
    }

    switch (buf[0]) {
    case 'y':
    case 'Y':
        return 1;
    }
    return 0;
}

AUG_EXTERNC int
aug_insertstream_(aug_mar_t mar, FILE* stream)
{
    char buf[AUG_MAXLINE];
    while (!feof(stream)) {

        size_t size = fread(buf, 1, sizeof(buf), stdin);
        if (size)
            aug_writemar(mar, buf, (unsigned)size);
    }
    return 0;
}

AUG_EXTERNC ssize_t
aug_readline_(char* buf, size_t size, FILE* stream)
{
    char* p = fgets(buf, (int)size, stream);
    if (!p) {

        if (feof(stream))
            return -2;

        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    if (!(p = strchr(buf, '\n'))) {

        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EPARSE,
                       AUG_MSG("newline character expected"));
        return -1;
    }

    *p = '\0';
    return (ssize_t)(p - buf);
}

AUG_EXTERNC int
aug_streamset_(aug_mar_t mar, FILE* stream)
{
    char buf[AUG_MAXLINE];
    struct aug_field field;

    for (;;) {

        switch (aug_readline_(buf, sizeof(buf), stream)) {
        case -1:
            return -1;
        case AUG_EENDOF:
            goto eof;
        }

        if (-1 == aug_atofield_(&field, buf))
            return -1;

        if (-1 == aug_setfield(mar, &field, NULL))
            return -1;
    }
 eof:
    return 0;
}

AUG_EXTERNC int
aug_writevalue_(FILE* stream, const void* value, size_t size)
{
    if (size != fwrite(value, 1, size, stream)
        || 1 != fwrite(&NL_, 1, 1, stream)) {

        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }

    return 0;
}
