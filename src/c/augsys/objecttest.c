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
    aug_initbasicctx();

    if (!(file = aug_createfile(aug_tlx, "filetest.txt",
                                O_CREAT | O_TRUNC | O_RDWR, 0666))) {
        aug_perrinfo(aug_tlx, "failed to open file");
        aug_term();
        return 1;
    }

    stream = aug_cast(file, aug_streamid);
    aug_release(file);

    aug_write(stream, "test\n", 5);
    aug_release(stream);

    aug_term();
    return 0;
}
