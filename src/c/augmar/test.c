/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augmar/mar.h"

int
main(int argc, char* argv[])
{
    struct aug_field field;
    aug_mar_t mar = aug_createmar();

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
