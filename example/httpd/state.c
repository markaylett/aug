/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#include "state.h"

static const char rcsid[] = "$Id$";

#include "message.h"

#include "augnet/http.h"
#include "augnet/types.h"
#include "augnet/utility.h"

#include "augutil/var.h"

#include "augsys/defs.h"  /* AUG_MAXLINE */
#include "augsys/errinfo.h"
#include "augsys/errno.h" /* ENOMEM */
#include "augsys/log.h"
#include "augsys/uio.h"
#include "augsys/unistd.h"

#include <assert.h>
#include <stdlib.h>       /* malloc() */
#include <string.h>       /* strlen() */

#define BUFSIZE_ 4096

struct aug_state_ {
    aug_request_t request_;
    struct aug_var arg_;
    struct {
        aug_httpparser_t parser_;
        aug_strbuf_t initial_;
        aug_mar_t mar_;
    } in_;
    struct {
        struct aug_messages pending_;
        aug_strbuf_t header_;     /* Current, formatted header. */
        struct iovec iovec_[2]; /* Header and body. */
        struct aug_buf buf_;
    } out_;
};

static int
format_(aug_strbuf_t* header, const char* initial, aug_mar_t mar)
{
    struct aug_field field;
    size_t ord = 0;

    aug_setstrbufs(header, initial);
    aug_catstrbufs(header, "\r\n");

    if (mar) {

        while (AUG_RETNOMATCH != aug_getfield(mar, &field, ord++)) {

            aug_catstrbufs(header, field.name_);
            if (0 < field.size_) {
                aug_catstrbufsn(header, ": ", 2);
                aug_catstrbufsn(header, field.value_, field.size_);
                aug_catstrbufsn(header, "\r\n", 2);
            } else
                aug_catstrbufsn(header, ":\r\n", 3);
        }
    }

    aug_catstrbufsn(header, "\r\n", 2);
    return 0;
}

static int
prepare_(aug_state_t state)
{
    struct aug_message* next = AUG_FIRST(&state->out_.pending_);
    assert(next);

    if (!state->out_.header_)
        state->out_.header_ = aug_createstrbuf(0);

    format_(&state->out_.header_, aug_getstr(next->initial_), next->mar_);

    AUG_DEBUG0("response: [%s]", aug_getstr(state->out_.header_));

    state->out_.buf_.iov_ = state->out_.iovec_;
    state->out_.buf_.size_ = 1;

    state->out_.iovec_[0].iov_base = (void*)aug_getstr(state->out_.header_);
    state->out_.iovec_[0].iov_len = aug_strbuflen(state->out_.header_);

    if (next->mar_) {
        unsigned size;
        state->out_.iovec_[1].iov_base
            = (void*)aug_content(next->mar_, &size);
        if (size) {
            state->out_.iovec_[1].iov_len = size;
            ++state->out_.buf_.size_;
        }
    }
    return 0;
}

static void
setinitial_(const struct aug_var* arg, const char* initial)
{
    aug_state_t state = aug_getvarp(arg);
    if (!state->in_.initial_)
        state->in_.initial_ = aug_createstrbuf(AUG_MAXLINE);
    aug_setstrbufs(&state->in_.initial_, initial);
}

static void
setfield_(const struct aug_var* arg, const char* name, const char* value)
{
    aug_state_t state = aug_getvarp(arg);
    struct aug_field field;

    field.name_ = name;
    field.value_ = value;
    field.size_ = strlen(value);

    if (!state->in_.mar_)
        state->in_.mar_ = aug_createmar();
    aug_setfield(state->in_.mar_, &field, NULL);
}

static void
setcsize_(const struct aug_var* arg, unsigned csize)
{
    aug_state_t state = aug_getvarp(arg);
    if (!state->in_.mar_)
        state->in_.mar_ = aug_createmar();
    aug_truncatemar(state->in_.mar_, csize);
    aug_seekmar(state->in_.mar_, AUG_SET, 0);
}

static void
cdata_(const struct aug_var* arg, const void* buf, unsigned size)
{
    aug_state_t state = aug_getvarp(arg);
    if (!state->in_.mar_)
        state->in_.mar_ = aug_createmar();
    aug_writemar(state->in_.mar_, buf, size);
}

static void
end_(const struct aug_var* arg, int commit)
{
    aug_state_t state = aug_getvarp(arg);
    if (commit) {

        static struct aug_messages messages = AUG_HEAD_INITIALIZER(messages);

        AUG_DEBUG0("request received");

        if (!state->in_.initial_)
            return; /* Blank line. */

        (*state->request_)(&state->arg_, aug_getstr(state->in_.initial_),
                           state->in_.mar_, &messages);
        AUG_CONCAT(&state->out_.pending_, &messages);
    }

    if (state->in_.initial_) {
        aug_freestrbuf(state->in_.initial_);
        state->in_.initial_ = NULL;
    }

    if (state->in_.mar_) {
        aug_releasemar(state->in_.mar_);
        state->in_.mar_ = NULL;
    }
}

const struct aug_httphandlers handlers_ = {
    setinitial_,
    setfield_,
    setcsize_,
    cdata_,
    end_
};

aug_state_t
aug_createstate(aug_request_t request, const struct aug_var* arg)
{
    aug_state_t state = malloc(sizeof(struct aug_state_));
    struct aug_var local;

    if (!state) {
        aug_setposixerrinfo(NULL, __FILE__, __LINE__, ENOMEM);
        return NULL;
    }

    state->request_ = request;
    aug_setvar(&state->arg_, arg);
    if (!(state->in_.parser_
          = aug_createhttpparser(AUG_MAXLINE, &handlers_,
                                 aug_setvarp(&local, state, NULL))))
        goto fail;

    state->in_.initial_ = NULL;
    state->in_.mar_ = NULL;
    AUG_INIT(&state->out_.pending_);
    state->out_.header_ = NULL;
    return state;

 fail:
    free(state);
    return NULL;
}

int
aug_freestate(aug_state_t state)
{
    struct aug_message* it;

    aug_freehttpparser(state->in_.parser_);

    if (state->in_.initial_)
        aug_freestrbuf(state->in_.initial_);

    if (state->in_.mar_)
        aug_releasemar(state->in_.mar_);

    AUG_FOREACH(it, &state->out_.pending_) {

        /* An initial line will always be present. */

        assert(it->initial_);
        aug_freestrbuf(it->initial_);

        if (it->mar_)
            aug_releasemar(it->mar_);
    }

    aug_freemessages(&state->out_.pending_);

    if (state->out_.header_)
        aug_freestrbuf(state->out_.header_);

    aug_freevar(&state->arg_);
    free(state);
    return 0;
}

int
aug_sendresponse(aug_state_t state, struct aug_message* message)
{
    AUG_INSERT_TAIL(&state->out_.pending_, message);
    return 0;
}

int
aug_pending(aug_state_t state)
{
    return !AUG_EMPTY(&state->out_.pending_);
}

ssize_t
aug_readsome(aug_state_t state, int fd)
{
    char buf[BUFSIZE_ + 1];
    int pending;
    ssize_t ret = aug_read(fd, buf, BUFSIZE_);

    if (0 < ret) {

#if !defined(NDEBUG)
        buf[ret] = '\0';
        AUG_DEBUG0("request: [%s]", buf);
#endif /* !NDEBUG */

        pending = !AUG_EMPTY(&state->out_.pending_);

        if (-1 == aug_parsehttp(state->in_.parser_, buf, ret))
            return -1;

        if (!pending && !AUG_EMPTY(&state->out_.pending_))
            prepare_(state);
    }

    return ret;
}

ssize_t
aug_writesome(aug_state_t state, int fd)
{
    ssize_t ret = aug_writev(fd, state->out_.buf_.iov_,
                             state->out_.buf_.size_);
    if (-1 != ret) {

        aug_addbuf(&state->out_.buf_, ret);

        if (0 == state->out_.buf_.size_) {

            struct aug_message* message = AUG_FIRST(&state->out_.pending_);
            AUG_REMOVE_HEAD(&state->out_.pending_);

            aug_freestrbuf(message->initial_);
            if (message->mar_)
                aug_releasemar(message->mar_);

            aug_freemessage(message);
            if (!AUG_EMPTY(&state->out_.pending_))
                prepare_(state);
        }
    }

    return ret;
}
