/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef MPLEXD_MESSAGE_HPP
#define MPLEXD_MESSAGE_HPP

#include "augutil/list.h"
#include "augutil/dstr.h"
#include "augmar/mar.h"

struct aug_message {
    AUG_ENTRY(aug_message);
    aug_dstr_t initial_;
    aug_mar_t mar_;
};

AUG_HEAD(aug_messages, aug_message);

struct aug_message*
aug_createmessage(aug_dstr_t initial, aug_mar_t mar);

int
aug_freemessages(struct aug_messages* messages);

int
aug_freemessage(struct aug_message* message);

#endif /* MPLEXD_MESSAGE_HPP */
