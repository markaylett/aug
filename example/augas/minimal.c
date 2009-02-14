/*
  Copyright (c) 2004-2009, Mark Aylett <mark.aylett@gmail.com>

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

static void
stop_(void)
{
    mod_writelog(MOD_LOGINFO, "stop_()");
}

static mod_bool
start_(struct mod_session* session)
{
    mod_writelog(MOD_LOGINFO, "start_()");
    return MOD_TRUE;
}

static void
reconf_(void)
{
    mod_writelog(MOD_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, struct aug_object_* ob)
{
    mod_writelog(MOD_LOGINFO, "event_()");
}

static void
closed_(const struct mod_handle* sock)
{
    mod_writelog(MOD_LOGINFO, "closed_()");
}

static void
teardown_(const struct mod_handle* sock)
{
    mod_writelog(MOD_LOGINFO, "teardown_()");
    mod_shutdown(sock->id_, 0);
}

static mod_bool
accepted_(struct mod_handle* sock, const char* name)
{
    mod_writelog(MOD_LOGINFO, "accepted_()");
    return MOD_TRUE;
}

static void
connected_(struct mod_handle* sock, const char* name)
{
    mod_writelog(MOD_LOGINFO, "connected_()");
}

static mod_bool
auth_(const struct mod_handle* sock, const char* subject, const char* issuer)
{
    mod_writelog(MOD_LOGINFO, "auth_()");
    return MOD_TRUE;
}

static void
recv_(const struct mod_handle* sock, const void* buf, size_t len)
{
    mod_writelog(MOD_LOGINFO, "recv_()");
    mod_send(sock->id_, buf, len);
}

static void
error_(const struct mod_handle* sock, const char* desc)
{
}

static void
rdexpire_(const struct mod_handle* sock, unsigned* ms)
{
    mod_writelog(MOD_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct mod_handle* sock, unsigned* ms)
{
    mod_writelog(MOD_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct mod_handle* timer, unsigned* ms)
{
    mod_writelog(MOD_LOGINFO, "expire_()");
}

static const struct mod_module module_ = {
    stop_,
    start_,
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

static const struct mod_module*
init_(const char* name)
{
    mod_writelog(MOD_LOGINFO, "init_()");
    return &module_;
}

static void
term_(void)
{
    mod_writelog(MOD_LOGINFO, "term_()");
}

MOD_ENTRYPOINTS(init_, term_)
