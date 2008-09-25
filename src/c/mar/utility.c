/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "mar/utility.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augctx/base.h"
#include "augctx/errinfo.h"

#include <errno.h>
#include <string.h>

static const char NL_ = '\n';

AUG_EXTERNC aug_result
aug_atofield_(struct aug_field* field, char* src)
{
    /* Split field in the form of "name=value". */

    char* value = strchr(src, '=');
    size_t size;

    if (NULL == value) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("empty value part"));
        return AUG_FAILERROR;
    }

    size = value - src;
    *value = '\0';

    field->name_ = src;
    field->value_ = ++value;
    field->size_ = (unsigned)strlen(value);
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_bool
aug_confirm_(const char* prompt)
{
    char buf[3];

    printf("%s? (y or [n]): ", prompt);
    fflush(stdout);

    switch (AUG_RESULT(aug_readline_(buf, sizeof(buf), stdin))) {
    case 1: /* One character. */
        break;
    default:
        return AUG_FALSE;
    }

    switch (buf[0]) {
    case 'y':
    case 'Y':
        return AUG_TRUE;
    }
    return AUG_FALSE;
}

AUG_EXTERNC aug_result
aug_insertstream_(aug_mar_t mar, FILE* stream)
{
    char buf[AUG_MAXLINE];
    while (!feof(stream)) {

        size_t size = fread(buf, 1, sizeof(buf), stdin);
        if (size)
            aug_verify(aug_writemar(mar, buf, (unsigned)size));
    }
    return AUG_SUCCESS;
}

AUG_EXTERNC aug_rsize
aug_readline_(char* buf, size_t size, FILE* stream)
{
    char* p = fgets(buf, (int)size, stream);
    if (!p) {

        if (feof(stream))
            return AUG_FAILNONE;

        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);
    }

    if (!(p = strchr(buf, '\n'))) {

        aug_seterrinfo(aug_tlerr, __FILE__, __LINE__, "aug", AUG_EPARSE,
                       AUG_MSG("newline character expected"));
        return AUG_FAILERROR;
    }

    *p = '\0';
    return AUG_MKRESULT((ssize_t)(p - buf));
}

AUG_EXTERNC aug_result
aug_streamset_(aug_mar_t mar, FILE* stream)
{
    char buf[AUG_MAXLINE];
    struct aug_field field;

    for (;;) {

        aug_rsize rsize = aug_readline_(buf, sizeof(buf), stream);
        if (AUG_ISFAIL(rsize)) {

            if (AUG_ISNONE(rsize))
                break;

            return rsize;
        }

        /* Split name/value. */

        aug_verify(aug_atofield_(&field, buf));
        aug_verify(aug_setfield(mar, &field, NULL));
    }

    return AUG_SUCCESS;
}

AUG_EXTERNC aug_result
aug_writevalue_(FILE* stream, const void* value, size_t size)
{
    if (size != fwrite(value, 1, size, stream)
        || 1 != fwrite(&NL_, 1, 1, stream))
        return aug_setposixerrinfo(aug_tlerr, __FILE__, __LINE__, errno);

    return AUG_SUCCESS;
}
