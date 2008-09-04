/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/

#include "augmar.h"
#include "augctx.h"

int
main(int argc, char* argv[])
{
    aug_mpool* mpool;
    struct aug_field field;
    aug_mar_t mar;

    if (aug_autobasictlx() < 0)
        return 1;

    mpool = aug_getmpool(aug_tlx);
    mar = aug_createmar(mpool);
    aug_release(mpool);

    field.name_ = "name";
    field.value_ = "Mark";
    field.size_ = 4;
    aug_setfield(mar, &field, NULL);

    field.name_ = "age";
    field.value_ = "33";
    field.size_ = 2;
    aug_setfield(mar, &field, NULL);

    aug_releasemar(mar);
    return 0;
}
