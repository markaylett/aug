/* Copyright (c) 2004-2006, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#ifndef HTTPD_STATE_H
#define HTTPD_STATE_H

#include "augsys/types.h"

#include "augmar/mar.h"

struct aug_message;
struct aug_messages;

typedef struct aug_state_* aug_state_t;
typedef int (*aug_request_t)(void*, const char*, aug_mar_t,
                             struct aug_messages*);

aug_state_t
aug_createstate(aug_request_t request, void* arg);

int
aug_freestate(aug_state_t state);

int
aug_sendresponse(aug_state_t state, struct aug_message* message);

int
aug_pending(aug_state_t state);

ssize_t
aug_readsome(aug_state_t state, int fd);

ssize_t
aug_writesome(aug_state_t state, int fd);

#endif /* HTTPD_STATE_H */
