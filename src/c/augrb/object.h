/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef AUGRB_OBJECT_H
#define AUGRB_OBJECT_H

#include "augmod.h"
#include "augext/blob.h"

#if !defined(_WIN32)
# include <unistd.h>
#else /* _WIN32 */
# define HAVE_ISINF 1
# if !defined(_MSC_VER)
#  define _MSC_VER 1200
# else /* _MSC_VER */
#  pragma comment(lib, "msvcrt-ruby18.lib")
# endif /* _MSC_VER */
char*
rb_w32_getcwd(char* buffer, int size);
#endif /* _WIN32 */

#include <ruby.h>

AUG_INTERFACE(augrb_blob);

struct augrb_blobvtbl {
    AUG_VTBL(augrb_blob);
    VALUE (*get_)(augrb_blob*);
};

aug_blob*
augrb_createblob(VALUE rbobj);

VALUE
augrb_getblob(aug_object* ob);

#endif /* AUGRB_OBJECT_H */
