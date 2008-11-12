/* Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>
   See the file COPYING for copying permission.
*/
#include "augutil.h"
#include "augctx.h"

#include <stdio.h>
#include <stdlib.h> /* exit() */

int
main(int argc, char* argv[])
{
    aug_mpool* mp;
    aug_xstr_t xs;
    const char* s;
    int i;

    if (AUG_ISFAIL(aug_autobasictlx()))
        return 1;

    mp = aug_getmpool(aug_tlx);
    aug_check(mp);

    xs = aug_createxstr(mp, 0);
    aug_check(xs);

    s = aug_xstr(xs);

    /* Append each letter of alphabet. */

    for (i = 0; i < 26; ++i) {
        aug_check(i == strlen(aug_xstr(xs)));
        aug_check(AUG_ISSUCCESS(aug_xstrcatc(xs, 'A' + i)));
    }

    /* Verify content. */

    aug_check(0 == strcmp(aug_xstr(xs), "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

    /* Re-allocation should _not_ have occurred. */

    aug_check(s == aug_xstr(xs));

    /* Copy to self. */

    aug_xstrcpy(xs, xs);
    aug_check(0 == strcmp(aug_xstr(xs), "ABCDEFGHIJKLMNOPQRSTUVWXYZ"));

    /* Force re-allocation. */

    for (i = 0; i < 10; ++i)
        aug_xstrcat(xs, xs);

    /* Re-allocation should have occurred. */

    aug_check(s != aug_xstr(xs));

    aug_destroyxstr(xs);
    aug_release(mp);
    return 0;
}
