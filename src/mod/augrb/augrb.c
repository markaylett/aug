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
#include <ctype.h>  /* tolower() */

#include "object.h" /* Redefines _MSC_VER. */

/* The bit fields indicate those functions implemented by the session. */

struct import_ {
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
    mod_bool open_;
};

/* Cache ids to avoid repeated lookups. */

static ID stopid_, startid_, reconfid_, eventid_, closedid_,
    teardownid_, acceptedid_, connectedid_, authid_, recvid_,
    errorid_, rdexpireid_, wrexpireid_, expireid_;

static VALUE maugrb_, chandle_, cerror_;

/* True if exception was thrown during last call to protect_(). */

static mod_bool except_ = MOD_FALSE;

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
    except_ = MOD_TRUE;
    except = rb_funcall(except, rb_intern("to_s"), 0);
    mod_writelog(MOD_LOGERROR, "%s", StringValuePtr(except));
    return Qnil;
}

static VALUE
protect_(VALUE (*body)(), VALUE args)
{
    VALUE ret;
    except_ = MOD_FALSE;
    ret = rb_rescue2(body, args, dorescue_, Qnil, rb_eException, (VALUE)0);
    return ret;
}

static VALUE
dofuncall_(VALUE module, VALUE args)
{
    return Qnil == args
        ? rb_funcall(module, u_.id_, 0)
        : rb_apply(module, u_.id_, args);
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

static VALUE
funcall4_(ID id, VALUE arg1, VALUE arg2, VALUE arg3, VALUE arg4)
{
    VALUE args = rb_ary_new3(4, arg1, arg2, arg3, arg4);
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

/* AugRb::Handle functions. */

static void
checkhandle_(VALUE handle)
{
    if (!rb_obj_is_kind_of(handle, chandle_))
        rb_raise(rb_eTypeError,
                 "wrong argument type %s (expected AugRb::Handle)",
                 rb_obj_classname(handle));
}

static mod_id
checkid_(VALUE handle)
{
    checkhandle_(handle);
    return FIX2UINT(rb_iv_get(handle, "@id"));
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
    mod_id lhs, rhs;
    int ret;

    checkhandle_(other);

    lhs = FIX2UINT(rb_iv_get(self, "@id"));
    rhs = FIX2UINT(rb_iv_get(other, "@id"));

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
    sprintf(sz, "#<AugRb::Handle:%lx,id=%u>", self,
            FIX2UINT(rb_iv_get(self, "@id")));
    return rb_str_new2(sz);
}

static void
termimport_(struct import_* import)
{
    /* Garbage collector takes care of rest. */
}

static mod_bool
initimport_(struct import_* import, const char* sname)
{
    VALUE module;

    /* Do this first - more likely to fail than malloc(). */

    u_.sname_ = sname;
    module = loadmodule_(sname);

    if (except_)
        return MOD_FALSE;

    import->module_ = module;

    /* Determine which functions the session has implemented. */

    import->stop_
        = rb_respond_to(import->module_, stopid_) ? 1 : 0;
    import->start_
        = rb_respond_to(import->module_, startid_) ? 1 : 0;
    import->reconf_
        = rb_respond_to(import->module_, reconfid_) ? 1 : 0;
    import->event_
        = rb_respond_to(import->module_, eventid_) ? 1 : 0;
    import->closed_
        = rb_respond_to(import->module_, closedid_) ? 1 : 0;
    import->teardown_
        = rb_respond_to(import->module_, teardownid_) ? 1 : 0;
    import->accepted_
        = rb_respond_to(import->module_, acceptedid_) ? 1 : 0;
    import->connected_
        = rb_respond_to(import->module_, connectedid_) ? 1 : 0;
    import->auth_
        = rb_respond_to(import->module_, authid_) ? 1 : 0;
    import->recv_
        = rb_respond_to(import->module_, recvid_) ? 1 : 0;
    import->error_
        = rb_respond_to(import->module_, errorid_) ? 1 : 0;
    import->rdexpire_
        = rb_respond_to(import->module_, rdexpireid_) ? 1 : 0;
    import->wrexpire_
        = rb_respond_to(import->module_, wrexpireid_) ? 1 : 0;
    import->expire_
        = rb_respond_to(import->module_, expireid_) ? 1 : 0;

    import->open_ = MOD_FALSE;
    return MOD_TRUE;
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
        rb_raise(cerror_, mod_geterror());

    return Qnil;
}

static VALUE
stopall_(VALUE self)
{
    if (mod_stopall() < 0)
        rb_raise(cerror_, mod_geterror());

    return Qnil;
}

static VALUE
post_(int argc, VALUE* argv, VALUE self)
{
    VALUE id, to, type, user;
    aug_blob* blob = NULL;
    mod_result result;

    rb_scan_args(argc, argv, "31", &id, &to, &type, &user);

    /* Type-check now to ensure string operations succeed. */

    Check_Type(id, T_FIXNUM);
    Check_Type(to, T_STRING);
    Check_Type(type, T_STRING);

    if (user != Qnil)
        blob = augrb_createblob(StringValue(user));
    result = mod_post(RSTRING(to)->ptr, RSTRING(type)->ptr, FIX2UINT(id),
                      (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (result < 0)
        rb_raise(cerror_, mod_geterror());

    return Qnil;
}

static VALUE
dispatch_(int argc, VALUE* argv, VALUE self)
{
    VALUE id, to, type, user;
    aug_blob* blob = NULL;
    mod_result result;

    rb_scan_args(argc, argv, "31", &id, &to, &type, &user);

    Check_Type(id, T_FIXNUM);
    Check_Type(to, T_STRING);
    Check_Type(type, T_STRING);

    if (user != Qnil)
        blob = augrb_createblob(StringValue(user));
    result = mod_dispatch(RSTRING(to)->ptr, RSTRING(type)->ptr, FIX2UINT(id),
                          (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (result < 0)
        rb_raise(cerror_, mod_geterror());

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
shutdown_(VALUE self, VALUE sock, VALUE flags)
{
    mod_id cid = checkid_(sock);

    if (mod_shutdown(cid, NUM2UINT(flags)) < 0)
        rb_raise(cerror_, mod_geterror());

    return Qnil;
}

static VALUE
tcpconnect_(int argc, VALUE* argv, VALUE self)
{
    VALUE host, serv, sslctx, user;
    const char* ptr;
    augrb_box* box;
    mod_rint cid;
    VALUE sock;

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

    box = augrb_boxhandle(0, user);
    cid = mod_tcpconnect(RSTRING(host)->ptr, RSTRING(serv)->ptr, ptr,
                         (aug_object*)box);
    aug_release(box);

    if (cid < 0)
        rb_raise(cerror_, mod_geterror());

    sock = box->vtbl_->unbox_(box);
    rb_iv_set(sock, "@id", INT2FIX(cid));
    return sock;
}

static VALUE
tcplisten_(int argc, VALUE* argv, VALUE self)
{
    VALUE host, serv, sslctx, user;
    const char* ptr;
    augrb_box* box;
    mod_rint cid;
    VALUE sock;

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

    box = augrb_boxhandle(0, user);
    cid = mod_tcplisten(RSTRING(host)->ptr, RSTRING(serv)->ptr, ptr,
                        (aug_object*)box);
    aug_release(box);

    if (cid < 0)
        rb_raise(cerror_, mod_geterror());

    sock = box->vtbl_->unbox_(box);
    rb_iv_set(sock, "@id", INT2FIX(cid));
    return sock;
}

static VALUE
send_(VALUE self, VALUE sock, VALUE buf)
{
    aug_blob* blob;
    mod_id cid = checkid_(sock);
    mod_result result;

    blob = augrb_createblob(StringValue(buf));
    result = mod_sendv(cid, blob);
    aug_release(blob);

    if (result < 0)
        rb_raise(cerror_, mod_geterror());

    return Qnil;
}

static VALUE
setrwtimer_(VALUE self, VALUE sock, VALUE ms, VALUE flags)
{
    mod_id cid = checkid_(sock);

    if (mod_setrwtimer(cid, NUM2UINT(ms), NUM2UINT(flags)) < 0)
        rb_raise(cerror_, mod_geterror());

    return Qnil;
}

static VALUE
resetrwtimer_(VALUE self, VALUE sock, VALUE ms, VALUE flags)
{
    mod_id cid = checkid_(sock);

    /* Return false if no such timer. */

    switch (mod_setrwtimer(cid, NUM2UINT(ms), NUM2UINT(flags))) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_geterror());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
cancelrwtimer_(VALUE self, VALUE sock, VALUE flags)
{
    mod_id cid = checkid_(sock);

    /* Return false if no such timer. */

    switch (mod_cancelrwtimer(cid, NUM2UINT(flags))) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_geterror());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
settimer_(int argc, VALUE* argv, VALUE self)
{
    VALUE ms, user;
    unsigned ui;
    augrb_box* box;
    mod_rint tid;
    VALUE timer;

    rb_scan_args(argc, argv, "11", &ms, &user);

    ui = NUM2UINT(ms);

    box = augrb_boxhandle(0, user);
    tid = mod_settimer(ui, (aug_object*)box);
    aug_release(box);

    if (tid < 0)
        rb_raise(cerror_, mod_geterror());

    timer = box->vtbl_->unbox_(box);
    rb_iv_set(timer, "@id", INT2FIX(tid));
    return timer;
}

static VALUE
resettimer_(VALUE self, VALUE timer, VALUE ms)
{
    mod_id tid = checkid_(timer);

    /* Return false if no such timer. */

    switch (mod_resettimer(tid, NUM2UINT(ms))) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_geterror());
    case MOD_FAILNONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
canceltimer_(VALUE self, VALUE timer)
{
    mod_id tid = checkid_(timer);

    /* Return false if no such timer. */

    switch (mod_canceltimer(tid)) {
    case MOD_FAILERROR:
        rb_raise(cerror_, mod_geterror());
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

struct impl_ {
    mod_session session_;
    int refs_;
    char name_[MOD_MAXNAME + 1];
    struct import_ import_;
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
    if (0 == --impl->refs_) {
        termimport_(&impl->import_);
        free(impl);
    }
}

static mod_bool
start_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    if (import->start_) {

        if (Qfalse == funcall1_(startid_, rb_str_new2(impl->name_))
            || except_)
            return MOD_FALSE;
    }

    import->open_ = MOD_TRUE;
    return MOD_TRUE;
}

static void
stop_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    if (import->open_ && import->stop_)
        funcall0_(stopid_);
}

static void
reconf_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    if (import->reconf_)
        funcall0_(reconfid_);
}

static void
event_(mod_session* ob_, const char* from, const char* type, mod_id id,
       aug_object* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob_);
    struct import_* import = &impl->import_;

    if (import->event_) {

        VALUE x = Qnil;
        if (ob) {

            /* Preference is augrb_blob type. */

            if (Qnil == (x = augrb_obtorb(ob))) {

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
        funcall4_(eventid_, INT2FIX(id), rb_str_new2(from),
                  rb_str_new2(type), x);
    }
}

static void
closed_(mod_session* ob, struct mod_handle* sock)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->closed_) {
        funcall1_(closedid_, augrb_obtorb(sock->ob_));
    }

    aug_assign(sock->ob_, NULL);
}

static void
teardown_(mod_session* ob, struct mod_handle* sock)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->teardown_)
        funcall1_(teardownid_, augrb_obtorb(sock->ob_));
    else
        mod_shutdown(sock->id_, 0);
}

static mod_bool
accepted_(mod_session* ob, struct mod_handle* sock, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    augrb_box* box;
    mod_bool ret = MOD_TRUE;

    assert(sock->ob_);

    /* On entry, sock->user_ is user data belonging to listener. */

    box = augrb_boxhandle(sock->id_, rb_iv_get(augrb_obtorb(sock->ob_),
                                               "@user"));

    /* Reject if function either returns false, or throws an exception. */

    if (import->accepted_) {

        VALUE user = box->vtbl_->unbox_(box);
        if (Qfalse == funcall2_(acceptedid_, user, rb_str_new2(name))
            || except_) {
            ret = MOD_FALSE;
            goto done;
        }
    }

    aug_assign(sock->ob_, (aug_object*)box);
 done:
    aug_release(box);
    return ret;
}

static void
connected_(mod_session* ob, struct mod_handle* sock, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->connected_)
        funcall2_(connectedid_, augrb_obtorb(sock->ob_), rb_str_new2(name));
}

static mod_bool
auth_(mod_session* ob, struct mod_handle* sock, const char* subject,
      const char* issuer)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    /* Reject if function either returns false, or throws an exception. */

    if (import->auth_)
        if (Qfalse == funcall3_(authid_, augrb_obtorb(sock->ob_),
                                rb_str_new2(subject), rb_str_new2(issuer))
            || except_)
            return MOD_FALSE;

    return MOD_TRUE;
}

static void
recv_(mod_session* ob, struct mod_handle* sock, const void* buf, size_t len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->recv_)
        funcall2_(recvid_, augrb_obtorb(sock->ob_),
                  rb_tainted_str_new(buf, (long)len));
}

static void
error_(mod_session* ob, struct mod_handle* sock, const char* desc)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->error_)
        funcall2_(errorid_, augrb_obtorb(sock->ob_), rb_str_new2(desc));
}

static void
rdexpire_(mod_session* ob, struct mod_handle* sock, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->rdexpire_) {
        VALUE ret = funcall2_(rdexpireid_, augrb_obtorb(sock->ob_),
                              INT2FIX(*ms));
        if (!except_ && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static void
wrexpire_(mod_session* ob, struct mod_handle* sock, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->wrexpire_) {
        VALUE ret = funcall2_(wrexpireid_, augrb_obtorb(sock->ob_),
                              INT2FIX(*ms));
        if (!except_ && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static void
expire_(mod_session* ob, struct mod_handle* timer, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(timer->ob_);

    if (import->expire_) {
        VALUE ret = funcall2_(expireid_, augrb_obtorb(timer->ob_),
                              INT2FIX(*ms));
        if (!except_ && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
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
    mod_writelog(MOD_LOGINFO, "initialising augrb module");
    ruby_init();

    /* Catch any exceptions. */

    protect_(initrb_, Qnil);

    /* Fail if exception thrown during initialisation. */

    if (except_) {
        ruby_finalize();
        return MOD_FALSE;
    }

    return MOD_TRUE;
}

static void
term_(void)
{
    mod_writelog(MOD_LOGINFO, "terminating augrb module");
    ruby_finalize();
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

    if (!initimport_(&impl->import_, sname)) {
        free(impl);
        return NULL;
    }

    return &impl->session_;
}

MOD_ENTRYPOINTS(init_, term_, create_)
