/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef HTTPD_MESSAGE_H
#define HTTPD_MESSAGE_H

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

#endif /* HTTPD_MESSAGE_H */
