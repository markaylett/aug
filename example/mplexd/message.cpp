/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "message.hpp"

#include "augsys/errno.h"
#include "augsys/lock.h"

#include <stdlib.h>

static struct aug_messages free_ = AUG_HEAD_INITIALIZER(free_);
AUG_ALLOCATOR(allocate_, &free_, aug_message, 64);

struct aug_message*
aug_createmessage(aug_dstr_t initial, aug_mar_t mar)
{
    struct aug_message* message;

    aug_lock();
    if (!(message = allocate_())) {
        aug_unlock();
        return NULL;
    }
    aug_unlock();

    message->initial_ = initial;
    message->mar_ = mar;
    return message;
}

int
aug_freemessages(struct aug_messages* messages)
{
    if (!AUG_EMPTY(messages)) {

        aug_lock();
        AUG_CONCAT(&free_, messages);
        aug_unlock();
    }
    return 0;
}

int
aug_freemessage(struct aug_message* message)
{
    aug_lock();
    AUG_INSERT_TAIL(&free_, message);
    aug_unlock();

    return 0;
}
