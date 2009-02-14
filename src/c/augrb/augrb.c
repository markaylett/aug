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
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include <assert.h>
#include <ctype.h>        /* tolower() */

#include "augrb/object.h" /* Redefines _MSC_VER. */

/* The bit fields indicate those functions implemented by the session. */

struct session_ {
    VALUE module_;
    int stop_ : 1;
    int start_ : 1;
    int reconf_ : 1;
    int event_ : 1;
    int closed_ : 1;
    int teardown_ : 1;
    int accepted_ : 1;
    int connected_ : 1;
    int auth_ : 1;
    int recv_ : 1;
    int error_ : 1;
    int rdexpire_ : 1;
    int wrexpire_ : 1;
    int expire_ : 1;
    int open_;
};

/* Cache ids to avoid repeated lookups. */

static ID stopid_, startid_, reconfid_, eventid_, closedid_,
    teardownid_, acceptedid_, connectedid_, authid_, recvid_,
    errorid_, rdexpireid_, wrexpireid_, expireid_;

static VALUE maugrb_, chandle_, cerror_;

/* True if exception was thrown during last call to protect_(). */

static int except_ = 0;

/* State used in Ruby callbacks. */

static union {

    /* See funcall_(). */

    ID id_;

    /* See loadmodule_(). */

    const char* sname_;

} u_;

static VALUE
dorescue_(VALUE unused, VALUE except)
{
    except_ = 1;
    except = rb_funcall(except, rb_intern("to_s"), 0);
    mod_writelog(MOD_LOGERROR, "%s", StringValuePtr(except));
    return Qnil;
}

static VALUE
protect_(VALUE (*body)(), VALUE args)
{
    VALUE ret;
    except_ = 0;
    ret = rb_rescue2(body, args, dorescue_, Qnil, rb_eException, (VALUE)0);
    return ret;
}

static VALUE
dofuncall_(VALUE args)
{
    struct session_* session = mod_getsession()->user_;
    return Qnil == args
        ? rb_funcall(session->module_, u_.id_, 0)
        : rb_apply(session->module_, u_.id_, args);
}

static VALUE
funcall_(ID id, VALUE args)
{
    u_.id_ = id;
    return protect_(dofuncall_, args);
}

static VALUE
funcall0_(ID id)
{
    return funcall_(id, Qnil);
}

static VALUE
funcall1_(ID id, VALUE arg1)
{
    VALUE args = rb_ary_new3(1, arg1);
    return funcall_(id, args);
}

static VALUE
funcall2_(ID id, VALUE arg1, VALUE arg2)
{
    VALUE args = rb_ary_new3(2, arg1, arg2);
    return funcall_(id, args);
}

static VALUE
funcall3_(ID id, VALUE arg1, VALUE arg2, VALUE arg3)
{
    VALUE args = rb_ary_new3(3, arg1, arg2, arg3);
    return funcall_(id, args);
}

static char*
lowercpy_(char* dst, const char* src)
{
    char* it;
    for (it = dst; '\0' != *src; ++it, ++src)
        *it = tolower(*src);
    *it = '\0';
    return dst;
}

static VALUE
doloadmodule_(VALUE unused)
{
    char* lower = alloca(strlen(u_.sname_) + 1);
    lowercpy_(lower, u_.sname_);

    mod_writelog(MOD_LOGINFO, "require [%s]", lower);

    rb_require(lower);
    return rb_const_get(rb_cObject, rb_intern(u_.sname_));
}

static VALUE
loadmodule_(const char* sname)
{
    u_.sname_ = sname;
    return protect_(doloadmodule_, Qnil);
}

/* VALUE-based var handling. */

static void
unregister_(VALUE* ptr)
{
    *ptr = Qnil; /* Belt and braces. */
    rb_gc_unregister_address(ptr);
    free(ptr);
}

static VALUE*
register_(VALUE value)
{
    VALUE* ptr = malloc(sizeof(VALUE));
    assert(ptr);
    *ptr = value;

    /* Prevent garbage collection. */

    rb_gc_register_address(ptr);
    return ptr;
}

/* AugRb::Handle functions. */

static void
checkhandle_(VALUE handle)
{
    if (!rb_obj_is_kind_of(handle, chandle_))
        rb_raise(rb_eTypeError,
                 "wrong argument type %s (expected AugRb::Handle)",
                 rb_obj_classname(handle));
}

static int
checkid_(VALUE handle)
{
    checkhandle_(handle);
    return FIX2INT(rb_iv_get(handle, "@id"));
}

static VALUE
inithandle_(VALUE self, VALUE id, VALUE user)
{
    Check_Type(id, T_FIXNUM);
    rb_iv_set(self, "@id", id);
    rb_iv_set(self, "@user", user);
    return self;
}

static VALUE
handleid_(VALUE self)
{
    return rb_iv_get(self, "@id");
}

static VALUE
handleuser_(VALUE self)
{
    return rb_iv_get(self, "@user");
}

static VALUE
sethandleuser_(VALUE self, VALUE user)
{
    rb_iv_set(self, "@user", user);
    return self;
}

static VALUE
cmphandle_(VALUE self, VALUE other)
{
    int lhs, rhs, ret;

    checkhandle_(other);

    lhs = FIX2INT(rb_iv_get(self, "@id"));
    rhs = FIX2INT(rb_iv_get(other, "@id"));

    if (lhs < rhs)
        ret = -1;
    else if (lhs > rhs)
        ret = 1;
    else
        ret = 0;

    return INT2FIX(ret);
}

static VALUE
handlehash_(VALUE self)
{
    return rb_iv_get(self, "@id");
}

static VALUE
handlestr_(VALUE self)
{
    char sz[64];
    sprintf(sz, "#<AugRb::Handle:%lx,id=%d>", self,
            FIX2INT(rb_iv_get(self, "@id")));
    return rb_str_new2(sz);
}

static VALUE
newhandle_(VALUE id, VALUE user)
{
    VALUE argv[2];
    argv[0] = id;
    argv[1] = user;
    return rb_class_new_instance(2, argv, chandle_);
}

static void
destroysession_(struct session_* session)
{
    /* Garbage collector takes care of rest. */

    free(session);
}

static struct session_*
createsession_(const char* sname)
{
    VALUE module;
    struct session_* session;

    /* Do this first - more likely to fail than malloc(). */

    u_.sname_ = sname;
    module = loadmodule_(sname);

    if (except_ || !(session = malloc(sizeof(struct session_))))
        return NULL;

    session->module_ = module;

    /* Determine which functions the session has implemented. */

    session->stop_
        = rb_respond_to(session->module_, stopid_) ? 1 : 0;
    session->start_
        = rb_respond_to(session->module_, startid_) ? 1 : 0;
    session->reconf_
        = rb_respond_to(session->module_, reconfid_) ? 1 : 0;
    session->event_
        = rb_respond_to(session->module_, eventid_) ? 1 : 0;
    session->closed_
        = rb_respond_to(session->module_, closedid_) ? 1 : 0;
    session->teardown_
        = rb_respond_to(session->module_, teardownid_) ? 1 : 0;
    session->accepted_
        = rb_respond_to(session->module_, acceptedid_) ? 1 : 0;
    session->connected_
        = rb_respond_to(session->module_, connectedid_) ? 1 : 0;
    session->auth_
        = rb_respond_to(session->module_, authid_) ? 1 : 0;
    session->recv_
        = rb_respond_to(session->module_, recvid_) ? 1 : 0;
    session->error_
        = rb_respond_to(session->module_, errorid_) ? 1 : 0;
    session->rdexpire_
        = rb_respond_to(session->module_, rdexpireid_) ? 1 : 0;
    session->wrexpire_
        = rb_respond_to(session->module_, wrexpireid_) ? 1 : 0;
    session->expire_
        = rb_respond_to(session->module_, expireid_) ? 1 : 0;

    session->open_ = 0;
    return session;
}

static VALUE
writelog_(VALUE self, VALUE level, VALUE msg)
{
    mod_writelog(NUM2INT(level), StringValuePtr(msg));
    return Qnil;
}

static VALUE
reconfall_(VALUE self)
{
    if (mod_reconfall() < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
stopall_(VALUE self)
{
    if (mod_stopall() < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
post_(int argc, VALUE* argv, VALUE self)
{
    VALUE to, type, user;
    aug_blob* blob = NULL;
    int ret;

    rb_scan_args(argc, argv, "21", &to, &type, &user);

    /* Type-check now to ensure string operations succeed. */

    Check_Type(to, T_STRING);
    Check_Type(type, T_STRING);

    if (user != Qnil)
        blob = augrb_createblob(StringValue(user));
    ret = mod_post(RSTRING(to)->ptr, RSTRING(type)->ptr, (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (ret < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
dispatch_(int argc, VALUE* argv, VALUE self)
{
    VALUE to, type, user;
    aug_blob* blob = NULL;
    int ret;

    rb_scan_args(argc, argv, "21", &to, &type, &user);

    Check_Type(to, T_STRING);
    Check_Type(type, T_STRING);

    if (user != Qnil)
        blob = augrb_createblob(StringValue(user));
    ret = mod_dispatch(RSTRING(to)->ptr, RSTRING(type)->ptr,
                        (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (ret < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
getenv_(int argc, VALUE* argv, VALUE self)
{
    VALUE name, def;
    const char* value;

    rb_scan_args(argc, argv, "11", &name, &def);

    Check_Type(name, T_STRING);

    if (!(value = mod_getenv(RSTRING(name)->ptr, NULL)))
        return def;

    return rb_tainted_str_new2(value);
}

static VALUE
getsession_(VALUE self)
{
    const struct mod_session* session;

    if (!(session = mod_getsession()))
        return Qnil;

    return rb_str_new2(session->name_);
}

static VALUE
shutdown_(VALUE self, VALUE sock, VALUE flags)
{
    int cid = checkid_(sock);

    if (mod_shutdown(cid, NUM2UINT(flags)) < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
tcpconnect_(int argc, VALUE* argv, VALUE self)
{
    VALUE host, serv, sslctx, user;
    const char* ptr;
    VALUE* sock;
    int cid;

    rb_scan_args(argc, argv, "22", &host, &serv, &sslctx, &user);

    /* Type-check now to ensure string operations succeed. */

    Check_Type(host, T_STRING);
    serv = StringValue(serv);
    if (sslctx == Qnil)
        ptr = NULL;
    else {
        Check_Type(sslctx, T_STRING);
        ptr = RSTRING(sslctx)->ptr;
    }

    sock = register_(newhandle_(INT2FIX(0), user));

    if ((cid = mod_tcpconnect(RSTRING(host)->ptr, RSTRING(serv)->ptr,
                              ptr, sock)) < 0) {
        unregister_(sock);
        rb_raise(cerror_, mod_error());
    }

    rb_iv_set(*sock, "@id", INT2FIX(cid));
    return *sock;
}

static VALUE
tcplisten_(int argc, VALUE* argv, VALUE self)
{
    VALUE host, serv, sslctx, user;
    const char* ptr;
    VALUE* sock;
    int cid;

    rb_scan_args(argc, argv, "22", &host, &serv, &sslctx, &user);

    /* Type-check now to ensure string operations succeed. */

    Check_Type(host, T_STRING);
    serv = StringValue(serv);
    if (sslctx == Qnil)
        ptr = NULL;
    else {
        Check_Type(sslctx, T_STRING);
        ptr = RSTRING(sslctx)->ptr;
    }

    sock = register_(newhandle_(INT2FIX(0), user));

    if ((cid = mod_tcplisten(RSTRING(host)->ptr, RSTRING(serv)->ptr,
                             ptr, sock)) < 0) {
        unregister_(sock);
        rb_raise(cerror_, mod_error());
    }

    rb_iv_set(*sock, "@id", INT2FIX(cid));
    return *sock;
}

static VALUE
send_(VALUE self, VALUE sock, VALUE buf)
{
    aug_blob* blob;
    int cid = checkid_(sock), ret;

    blob = augrb_createblob(StringValue(buf));
    ret = mod_sendv(cid, blob);
    aug_release(blob);

    if (ret < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
setrwtimer_(VALUE self, VALUE sock, VALUE ms, VALUE flags)
{
    int cid = checkid_(sock);

    if (mod_setrwtimer(cid, NUM2UINT(ms), NUM2UINT(flags)) < 0)
        rb_raise(cerror_, mod_error());

    return Qnil;
}

static VALUE
resetrwtimer_(VALUE self, VALUE sock, VALUE ms, VALUE flags)
{
    int cid = checkid_(sock);

    /* Return false if no such timer. */

    switch (mod_setrwtimer(cid, NUM2UINT(ms), NUM2UINT(flags))) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_error());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
cancelrwtimer_(VALUE self, VALUE sock, VALUE flags)
{
    int cid = checkid_(sock);

    /* Return false if no such timer. */

    switch (mod_cancelrwtimer(cid, NUM2UINT(flags))) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_error());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
settimer_(int argc, VALUE* argv, VALUE self)
{
    VALUE ms, user, timer;

    unsigned ui;
    aug_blob* blob;
    int tid;

    rb_scan_args(argc, argv, "11", &ms, &user);

    ui = NUM2UINT(ms);

    timer = newhandle_(INT2FIX(0), user);
    blob = augrb_createblob(timer);
    tid = mod_settimer(ui, (aug_object*)blob);
    aug_release(blob);

    if (tid < 0)
        rb_raise(cerror_, mod_error());

    rb_iv_set(timer, "@id", INT2FIX(tid));
    return timer;
}

static VALUE
resettimer_(VALUE self, VALUE timer, VALUE ms)
{
    int tid = checkid_(timer);

    /* Return false if no such timer. */

    switch (mod_resettimer(tid, NUM2UINT(ms))) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_error());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
canceltimer_(VALUE self, VALUE timer)
{
    int tid = checkid_(timer);

    /* Return false if no such timer. */

    switch (mod_canceltimer(tid)) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_error());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static void
setpath_(void)
{
    const char* s;
    char prev[1024], ruby[1024];

    /* Store working directory for later restoration. */

    getcwd(prev, sizeof(prev));

    /* Path may be relative to run directory. */

    if ((s = mod_getenv("rundir", NULL)))
        chdir(s);

    s = mod_getenv("module.augrb.rubypath", "ruby");
    mod_writelog(MOD_LOGDEBUG, "module.augrb.rubypath=[%s]", s);
    chdir(s);

    /* Append current directory. */

    getcwd(ruby, sizeof(ruby));
    ruby_incpush(ruby);

    /* Restore previous working directory. */

    chdir(prev);
}

static VALUE
initrb_(VALUE unused)
{
#if defined(NT)
    static int argc = 1;
    static char* argv[] = { "augrb", NULL };
    char** pp = argv;
    NtInitialize(&argc, &pp);
#endif /* NT */

    ruby_script("augrb");
    setpath_();

    /* Ids of session functions. */

    stopid_ = rb_intern("stop");
    startid_ = rb_intern("start");
    reconfid_ = rb_intern("reconf");
    eventid_= rb_intern("event");
    closedid_= rb_intern("closed");
    teardownid_= rb_intern("teardown");
    acceptedid_= rb_intern("accepted");
    connectedid_= rb_intern("connected");
    authid_= rb_intern("auth");
    recvid_= rb_intern("recv");
    errorid_= rb_intern("error");
    rdexpireid_= rb_intern("rdexpire");
    wrexpireid_= rb_intern("wrexpire");
    expireid_= rb_intern("expire");

    maugrb_ = rb_define_module("AugRb");
    chandle_ = rb_define_class_under(maugrb_, "Handle", rb_cObject);
    cerror_ = rb_define_class_under(maugrb_, "Error", rb_eStandardError);

    /* Logger constants. */

    rb_define_const(maugrb_, "LOGCRIT", INT2FIX(MOD_LOGCRIT));
    rb_define_const(maugrb_, "LOGERROR", INT2FIX(MOD_LOGERROR));
    rb_define_const(maugrb_, "LOGWARN", INT2FIX(MOD_LOGWARN));
    rb_define_const(maugrb_, "LOGNOTICE", INT2FIX(MOD_LOGNOTICE));
    rb_define_const(maugrb_, "LOGINFO", INT2FIX(MOD_LOGINFO));
    rb_define_const(maugrb_, "LOGDEBUG", INT2FIX(MOD_LOGDEBUG));

    /* Timer constants. */

    rb_define_const(maugrb_, "TIMRD", INT2FIX(MOD_TIMRD));
    rb_define_const(maugrb_, "TIMWR", INT2FIX(MOD_TIMWR));
    rb_define_const(maugrb_, "TIMRDWR", INT2FIX(MOD_TIMRDWR));

    /* Shutdown constants. */

    rb_define_const(maugrb_, "SHUTNOW", INT2FIX(MOD_SHUTNOW));

    /* Object methods. */

    rb_define_method(chandle_, "initialize", inithandle_, 2);
    rb_define_method(chandle_, "id", handleid_, 0);
    rb_define_method(chandle_, "user", handleuser_, 0);
    rb_define_method(chandle_, "user=", sethandleuser_, 1);

    rb_define_method(chandle_, "<=>", cmphandle_, 1);
    rb_define_method(chandle_, "hash", handlehash_, 0);
    rb_define_method(chandle_, "to_s", handlestr_, 0);

    rb_include_module(chandle_, rb_mComparable);

    /* Host module functions. */

    rb_define_module_function(maugrb_, "writelog", writelog_, 2);
    rb_define_module_function(maugrb_, "reconfall", reconfall_, 0);
    rb_define_module_function(maugrb_, "stopall", stopall_, 0);
    rb_define_module_function(maugrb_, "post", post_, -1);
    rb_define_module_function(maugrb_, "dispatch", dispatch_, -1);
    rb_define_module_function(maugrb_, "getenv", getenv_, -1);
    rb_define_module_function(maugrb_, "getsession", getsession_, 0);
    rb_define_module_function(maugrb_, "shutdown", shutdown_, 2);
    rb_define_module_function(maugrb_, "tcpconnect", tcpconnect_, -1);
    rb_define_module_function(maugrb_, "tcplisten", tcplisten_, -1);
    rb_define_module_function(maugrb_, "send", send_, 2);
    rb_define_module_function(maugrb_, "setrwtimer", setrwtimer_, 3);
    rb_define_module_function(maugrb_, "resetrwtimer", resetrwtimer_, 3);
    rb_define_module_function(maugrb_, "cancelrwtimer", cancelrwtimer_, 2);
    rb_define_module_function(maugrb_, "settimer", settimer_, -1);
    rb_define_module_function(maugrb_, "resettimer", resettimer_, 2);
    rb_define_module_function(maugrb_, "canceltimer", canceltimer_, 1);

    return Qnil;
}

static void
stop_(void)
{
    struct session_* session = mod_getsession()->user_;
    assert(session);

    if (session->open_ && session->stop_)
        funcall0_(stopid_);

    destroysession_(session);
}

static mod_bool
start_(struct mod_session* session)
{
    struct session_* local;
    if (!(local = createsession_(session->name_)))
        return MOD_FALSE;

    session->user_ = local;

    if (local->start_) {
        if (Qfalse == funcall1_(startid_, rb_str_new2(session->name_))
            || except_) {
            destroysession_(local);
            return MOD_FALSE;
        }
    }

    local->open_ = 1;
    return MOD_TRUE;
}

static void
reconf_(void)
{
    struct session_* session = mod_getsession()->user_;
    assert(session);

    if (session->reconf_)
        funcall0_(reconfid_);
}

static void
event_(const char* from, const char* type, aug_object* ob)
{
    struct session_* session = mod_getsession()->user_;
    assert(session);

    if (session->event_) {

        VALUE x = Qnil;
        if (ob) {

            /* Preference is augpy_blob type. */

            if (Qnil == (x = augrb_getblob(ob))) {

                /* Fallback to aug_blob type. */

                aug_blob* blob = aug_cast(ob, aug_blobid);
                if (blob) {

                    size_t size;
                    const void* data = aug_getblobdata(blob, &size);

                    /* Unsafe to release here. */

                    if (data)
                        x = rb_tainted_str_new(data, (long)size);

                    aug_release(blob);
                }
            }
        }
        funcall3_(eventid_, rb_str_new2(from), rb_str_new2(type), x);
    }
}

static void
closed_(const struct mod_handle* sock)
{
    struct session_* session = mod_getsession()->user_;
    assert(session);
    assert(sock->user_);

    if (session->closed_)
        funcall1_(closedid_, *(VALUE*)sock->user_);

    unregister_(sock->user_);
}

static void
teardown_(const struct mod_handle* sock)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    if (session->teardown_)
        funcall1_(teardownid_, user);
    else
        mod_shutdown(sock->id_, 0);
}

static mod_bool
accepted_(struct mod_handle* sock, const char* name)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);

    /* On entry, sock->user_ is user data belonging to listener. */

    user = newhandle_(INT2FIX(sock->id_),
                      rb_iv_get(*(VALUE*)sock->user_, "@user"));

    /* Reject if function either returns false, or throws an exception. */

    if (session->accepted_)
        if (Qfalse == funcall2_(acceptedid_, user, rb_str_new2(name))
            || except_)
            return MOD_FALSE;

    sock->user_ = register_(user);
    return MOD_TRUE;
}

static void
connected_(struct mod_handle* sock, const char* name)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    if (session->connected_)
        funcall2_(connectedid_, user, rb_str_new2(name));
}

static mod_bool
auth_(const struct mod_handle* sock, const char* subject, const char* issuer)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    /* Reject if function either returns false, or throws an exception. */

    if (session->auth_)
        if (Qfalse == funcall3_(authid_, user, rb_str_new2(subject),
                                rb_str_new2(issuer)) || except_)
            return MOD_FALSE;

    return MOD_TRUE;
}

static void
recv_(const struct mod_handle* sock, const void* buf, size_t len)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    if (session->recv_)
        funcall2_(recvid_, user, rb_tainted_str_new(buf, (long)len));
}

static void
error_(const struct mod_handle* sock, const char* desc)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    if (session->error_)
        funcall2_(errorid_, user, rb_str_new2(desc));
}

static void
rdexpire_(const struct mod_handle* sock, unsigned* ms)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    if (session->rdexpire_) {
        VALUE ret = funcall2_(rdexpireid_, user, INT2FIX(*ms));
        if (!except_ && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static void
wrexpire_(const struct mod_handle* sock, unsigned* ms)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    if (session->wrexpire_) {
        VALUE ret = funcall2_(wrexpireid_, user, INT2FIX(*ms));
        if (!except_ && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static void
expire_(const struct mod_handle* timer, unsigned* ms)
{
    struct session_* session = mod_getsession()->user_;
    VALUE user;
    assert(session);
    assert(timer->user_);
    user = augrb_getblob((aug_object*)timer->user_);

    if (session->expire_) {
        VALUE ret = funcall2_(expireid_, user, INT2FIX(*ms));
        if (!except_ && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
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
    mod_writelog(MOD_LOGINFO, "initialising augrb module");
    ruby_init();

    /* Catch any exceptions. */

    protect_(initrb_, Qnil);

    /* Fail if exception thrown during initialisation. */

    if (except_) {
        ruby_finalize();
        return NULL;
    }

    return &module_;
}

static void
term_(void)
{
    mod_writelog(MOD_LOGINFO, "terminating augrb module");
    ruby_finalize();
}

MOD_ENTRYPOINTS(init_, term_)
