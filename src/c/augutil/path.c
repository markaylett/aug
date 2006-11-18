/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGUTIL_BUILD
#include "augutil/path.h"

static const char rcsid[] = "$Id$";

#include "augsys/errinfo.h"
#include "augsys/limits.h" /* AUG_PATH_MAX */
#include "augsys/string.h"
#include "augsys/unistd.h"

#include <ctype.h>         /* isalpha() */
#include <errno.h>
#include <stdlib.h>        /* realpath() */
#include <string.h>        /* strlen() */

#if !defined(_WIN32)
# include <alloca.h>
# define IS_DIRSEP_(ch) ((ch) == '/')
#else /* _WIN32 */
# include <malloc.h>
# define IS_DIRSEP_(ch) ((ch) == '/' || (ch) == '\\')
#endif /* _WIN32 */

AUGUTIL_API const char*
aug_basename(const char* path)
{
    const char* base;

#if defined(_WIN32)
    if (isalpha(path[0]) && ':' == path[1])
        path += 2;
#endif /* _WIN32 */

    for (base = path; *path; ++path)
        if (IS_DIRSEP_(*path))
            base = path + 1;

    return base;
}

AUGUTIL_API int
aug_chdir(const char* path)
{
    if (-1 == chdir(path)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return -1;
    }
    return 0;
}

AUGUTIL_API char*
aug_getcwd(char* dst, size_t size)
{
#if !defined(_WIN32)
    if (!getcwd(dst, size)) {
#else /* _WIN32 */
    if (!getcwd(dst, (int)size)) {
#endif /* _WIN32 */
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }
    return dst;
}

AUGUTIL_API char*
aug_makepath(char* dst, const char* dir, const char* name, const char* ext,
             size_t size)
{
    size_t namelen = strlen(name);
    size_t extlen = strlen(ext);

    /* The directory part is optional. */

    if (dir && '\0' != *dir) {

        size_t dirlen = strlen(dir);

        /* Strip trailing separator from directory part. */

        if (IS_DIRSEP_(dir[dirlen - 1]))
            --dirlen;

        if (size < dirlen + namelen + extlen + 3) {
            aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                           AUG_MSG("buffer size exceeded"));
            return NULL;
        }

        /* Directory part. */

        memcpy(dst, dir, dirlen);
        dst[dirlen] = '/';
        dst += dirlen + 1;

    } else if (size < namelen + extlen + 2) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                       AUG_MSG("buffer size exceeded"));
        return NULL;
    }

    /* File name part. */

    memcpy(dst, name, namelen);
    dst[namelen] = '.';
    dst += namelen + 1;

    /* Extension part. */

    strcpy(dst, ext);
    return dst;
}

AUGUTIL_API char*
aug_realpath(char* dst, const char* src, size_t size)
{
#if !defined(_WIN32)
    int pathmax;
    char* buf;

    /* TODO: The following sequence attempts to provide a safe implementation
       of realpath().  Verify that this is indeed the case. */

    if (-1 == (pathmax = pathconf(src, _PC_PATH_MAX))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }

    if (!(buf = alloca(pathmax + 1))) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    if (!realpath(src, buf)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }

    if (size <= strlen(buf)) {
        aug_seterrinfo(NULL, __FILE__, __LINE__, AUG_SRCLOCAL, AUG_EBOUND,
                       AUG_MSG("buffer size exceeded"));
        return NULL;
    }

    aug_strlcpy(dst, buf, size);
    return dst;
#else /* _WIN32 */
    if (!_fullpath(dst, src, size)) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, errno);
        return NULL;
    }
    return dst;
#endif /* _WIN32 */
}
