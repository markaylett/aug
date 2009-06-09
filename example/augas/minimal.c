/*
  Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>

  This file is part of Aug written by Mark Aylett.

  Aug is released under the GPL with the additional exemption that compiling,
  linking, and/or using OpenSSL is allowed.

  Aug is free software; you can redistribute it and/or modify it under the
  terms of the GNU General Public License as published by the Free Software
  Foundation; either version 2 of the License, or (at your option) any later
  version.

  Aug is distributed in the hope that it will be useful, but WITHOUT ANY
  WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
  FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
  details.

  You should have received a copy of the GNU General Public License along with
  this program; if not, write to the Free Software Foundation, Inc., 51
  Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/
#define MOD_BUILD
#include "augmod.h"
#include "augses.h"

#include <string.h> /* strcmp() */

struct impl_ {
    mod_session session_;
    int refs_;
    char name_[MOD_MAXNAME + 1];
};

static void*
cast_(mod_session* ob, const char* id)
{
    if (AUG_EQUALID(id, aug_objectid) || AUG_EQUALID(id, mod_sessionid)) {
        aug_retain(ob);
        return ob;
    }
    return NULL;
}

static void
retain_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    ++impl->refs_;
}

static void
release_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    if (0 == --impl->refs_)
        free(impl);
}

static mod_bool
start_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: start_()", impl->name_);
    return MOD_TRUE;
}

static void
stop_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: stop_()", impl->name_);
}

static void
reconf_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: reconf_()", impl->name_);
}

static void
event_(mod_session* ob_, const char* from, const char* type, mod_id id,
       aug_object* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob_);
    mod_writelog(MOD_LOGINFO, "%s: event_()", impl->name_);
}

static void
closed_(mod_session* ob, struct mod_handle* sock)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: closed_()", impl->name_);
}

static void
teardown_(mod_session* ob, struct mod_handle* sock)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: teardown_()", impl->name_);
    mod_shutdown(sock->id_, 0);
}

static mod_bool
accepted_(mod_session* ob, struct mod_handle* sock, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: accepted_()", impl->name_);
    return MOD_TRUE;
}

static void
connected_(mod_session* ob, struct mod_handle* sock, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: connected_()", impl->name_);
}

static mod_bool
auth_(mod_session* ob, struct mod_handle* sock, const char* subject,
      const char* issuer)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: auth_()", impl->name_);
    return MOD_TRUE;
}

static void
recv_(mod_session* ob, struct mod_handle* sock, const void* buf, size_t len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: recv_()", impl->name_);
    mod_send(sock->id_, buf, len);
}

static void
error_(mod_session* ob, struct mod_handle* sock, const char* desc)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: error_()", impl->name_);
}

static void
rdexpire_(mod_session* ob, struct mod_handle* sock, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: rdexpire_()", impl->name_);
}

static void
wrexpire_(mod_session* ob, struct mod_handle* sock, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: wrexpire_()", impl->name_);
}

static void
expire_(mod_session* ob, struct mod_handle* timer, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    mod_writelog(MOD_LOGINFO, "%s: expire_()", impl->name_);
}

static const struct mod_sessionvtbl vtbl_ = {
    cast_,
    retain_,
    release_,
    start_,
    stop_,
    reconf_,
    event_,
    closed_,
    teardown_,
    accepted_,
    connected_,
    auth_,
    recv_,
    error_,
    rdexpire_,
    wrexpire_,
    expire_
};

static mod_bool
init_(const char* name)
{
    mod_writelog(MOD_LOGINFO, "init_()");
    return MOD_TRUE;
}

static void
term_(void)
{
    mod_writelog(MOD_LOGINFO, "term_()");
}

static mod_session*
create_(const char* sname)
{
    struct impl_* impl = malloc(sizeof(struct impl_));
    if (!impl)
        return NULL;

    impl->session_.vtbl_ = &vtbl_;
    impl->session_.impl_ = NULL;
    impl->refs_ = 1;

    strncpy(impl->name_, sname, sizeof(impl->name_));
    impl->name_[MOD_MAXNAME] = '\0';

    return &impl->session_;
}

MOD_ENTRYPOINTS(init_, term_, create_)
