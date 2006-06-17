/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "mar/utility.h"

#include "augsys/defs.h" /* AUG_BUFSIZE */
#include "augsys/errno.h"

#include <string.h>

static const char NL_ = '\n';

MAR_EXTERN int
aug_atofield_(struct aug_field* field, char* src)
{
    /* Split field in the form of "name=value". */

    char* value = strchr(src, '=');
    size_t size;

    if (NULL == value) {

        errno = EINVAL;
        return -1;
    }

    size = value - src;
    *value = '\0';

    field->name_ = src;
    field->value_ = ++value;
    field->size_ = strlen(value);
    return 0;
}

MAR_EXTERN int
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

MAR_EXTERN int
aug_insertstream_(aug_mar_t mar, FILE* stream)
{
    char buf[AUG_BUFSIZE];
    while (!feof(stream)) {

        size_t size = fread(buf, 1, sizeof(buf), stdin);
        if (size)
            aug_writemar(mar, buf, size);
    }
    return 0;
}

MAR_EXTERN ssize_t
aug_readline_(char* buf, size_t size, FILE* stream)
{
    char* p = fgets(buf, size, stream);
    if (!p) {

        if (feof(stream))
            return -2;

        return -1;
    }

    if (!(p = strchr(buf, '\n'))) {

        errno = EINVAL;
        return -1;
    }

    *p = '\0';
    return p - buf;
}

MAR_EXTERN int
aug_streamset_(aug_mar_t mar, FILE* stream)
{
    char buf[AUG_BUFSIZE];
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

MAR_EXTERN int
aug_writevalue_(FILE* stream, const void* value, size_t size)
{
    if (size != fwrite(value, 1, size, stream))
        return -1;

    if (1 != fwrite(&NL_, 1, 1, stream))
        return -1;

    return 0;
}
