/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "augsys/file.h"
#include "augsys/stream.h"
#include "augctx/base.h"

#include <fcntl.h>

int
main(int argc, char* argv[])
{
    aug_file* file;
    aug_stream* stream;
    char buf[100];
    aug_initbasicctx();

    if (!(file = aug_createfile(aug_tlx, "filetest.txt", O_CREAT | O_RDONLY)))
        aug_perrinfo(aug_tlx, "failed to open file");

    stream = aug_cast(file, aug_streamid);
    aug_release(file);

    aug_read(stream, buf, 100);
    aug_release(stream);

    aug_term();

    return 0;
}
