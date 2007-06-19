#include "augrt.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if defined(_WIN32)
# define HAVE_ISINF 1
# define _MSC_VER 1200
#endif /* _WIN32 */
#include <ruby.h>

#include <assert.h>

#include "augrt.h"
#include "ruby.h"

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
    int data_ : 1;
    int rdexpire_ : 1;
    int wrexpire_ : 1;
    int expire_ : 1;
    int authcert_ : 1;
    int open_;
};

/* Cache ids to avoid repeated lookups. */

static ID stopid_, startid_, reconfid_, eventid_, closedid_,
    teardownid_, acceptedid_, connectedid_, dataid_, rdexpireid_,
    wrexpireid_, expireid_, authcertid_;

static VALUE maugrt_, cobject_, cerror_;

static void
unregister_(VALUE* ptr)
{
    *ptr = Qnil;
    rb_gc_unregister_address(ptr);
    free(ptr);
}

static VALUE*
register_(VALUE value)
{
    VALUE* ptr = malloc(sizeof(VALUE));
    *ptr = value;
    rb_gc_register_address(ptr);
    return ptr;
}

static int
destroy_(void* arg)
{
    unregister_(arg);
    return 0;
}

static const void*
buf_(void* arg, size_t* size)
{
    if (size)
        *size = RSTRING(arg)->len;
    return RSTRING(arg)->ptr;
}

static const struct augrt_vartype vartype_ = {
    destroy_,
    buf_
};

static VALUE
initobject_(VALUE self, VALUE id, VALUE user)
{
    Check_Type(id, T_FIXNUM);
    rb_iv_set(self, "@id", id);
    rb_iv_set(self, "@user", user);
    return self;
}

static VALUE
objectid_(VALUE self)
{
    return rb_iv_get(self, "@id");
}

static VALUE
objectuser_(VALUE self)
{
    return rb_iv_get(self, "@user");
}

static VALUE
setobjectuser_(VALUE self, VALUE user)
{
    rb_iv_set(self, "@user", user);
    return self;
}

static VALUE
cmpobject_(VALUE self, VALUE other)
{
    int lhs, rhs, ret;

    Check_Type(other, TYPE(cobject_));

    lhs = FIX2INT(rb_iv_get(self, "@id"));
    rhs = FIX2INT(rb_iv_get(other, "@id"));

    if (lhs < rhs)
        ret = -1;
    else if (lhs > rhs)
        ret = 1;
    else
        ret = 0;
    return ret;
}

static VALUE
objecthash_(VALUE self)
{
    return rb_iv_get(self, "@id");
}

static VALUE
objectstr_(VALUE self)
{
    char sz[64];
    sprintf(sz, "#<Augrt::Object:%lx,id=%d>", self,
            FIX2INT(rb_iv_get(self, "@id")));
    return rb_str_new2(sz);
}

static VALUE
newobject_(VALUE id, VALUE user)
{
    VALUE argv[] = { id, user };
    return rb_class_new_instance(2, argv, cobject_);
}

static int
checkid_(VALUE object)
{
    Check_Type(object, TYPE(cobject_));
    return FIX2INT(objectid_(object));
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
loadmodule_(VALUE sname)
{
    char* lower = alloca(RSTRING(sname)->len + 1);
    lowercpy_(lower, StringValueCStr(sname));

    augrt_writelog(AUGRT_LOGINFO, "require '%s'", lower);

    rb_require(lower);
    return rb_const_get(rb_cObject, rb_to_id(sname));
}

static void
destroysession_(struct session_* session)
{
    free(session);
}

static struct session_*
createsession_(const char* sname)
{
    int except = 0;
    VALUE module = rb_protect(loadmodule_, rb_str_new2(sname), &except);
    struct session_* session;

    if (except || !(session = malloc(sizeof(struct session_)))) {
        augrt_writelog(AUGRT_LOGERROR, "failed to load session '%s'", sname);
        return NULL;
    }

    session->module_ = module;

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
    session->data_
        = rb_respond_to(session->module_, dataid_) ? 1 : 0;
    session->rdexpire_
        = rb_respond_to(session->module_, rdexpireid_) ? 1 : 0;
    session->wrexpire_
        = rb_respond_to(session->module_, wrexpireid_) ? 1 : 0;
    session->expire_
        = rb_respond_to(session->module_, expireid_) ? 1 : 0;
    session->authcert_
        = rb_respond_to(session->module_, authcertid_) ? 1 : 0;

    session->open_ = 0;
    return session;
}

static VALUE
writelog_(VALUE level, VALUE msg)
{
    augrt_writelog(NUM2INT(level), StringValueCStr(msg));
    return Qnil;
}

static VALUE
reconfall_(void)
{
    if (-1 == augrt_reconfall())
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static VALUE
stopall_(void)
{
    if (-1 == augrt_stopall())
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static VALUE
post_(int argc, VALUE *argv)
{
    VALUE to, type, buf;
    struct augrt_var var = { NULL, NULL };

    rb_scan_args(argc, argv, "21", &to, &type, &buf);

    Check_Type(to, T_STRING);
    Check_Type(type, T_STRING);

    if (buf != Qnil) {
        var.type_ = &vartype_;
        var.arg_ = register_(StringValue(buf));
    }

    if (-1 == augrt_post(StringValueCStr(to), StringValueCStr(type), &var)) {
        if (var.arg_)
            unregister_(var.arg_);
        rb_raise(cerror_, augrt_error());
    }

    return Qnil;
}

static VALUE
dispatch_(int argc, VALUE *argv)
{
    VALUE to, type, user;
    const void* ptr = NULL;
    size_t len = 0;

    rb_scan_args(argc, argv, "21", &to, &type, &user);

    if (user != Qnil) {
        user = StringValue(user);
        ptr = RSTRING(user)->ptr;
        len = RSTRING(user)->len;
    }

    if (-1 == augrt_dispatch(StringValueCStr(to), StringValueCStr(type),
                             ptr, len))
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static VALUE
getenv_(int argc, VALUE *argv)
{
    VALUE name, def;
    const char* value;

    rb_scan_args(argc, argv, "11", &name, &def);

    if (!(value = augrt_getenv(StringValueCStr(name), NULL)))
        return def;

    return rb_str_new2(value);
}

static VALUE
getsession_(void)
{
    const struct augrt_session* session;

    if (!(session = augrt_getsession()))
        return Qnil;

    return rb_str_new2(session->name_);
}

static VALUE
shutdown_(VALUE sock)
{
    Check_Type(sock, TYPE(cobject_));

    if (-1 == augrt_shutdown(FIX2INT(rb_iv_get(sock, "@id"))))
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static VALUE
tcpconnect_(int argc, VALUE* argv)
{
    VALUE host, serv, user;
    VALUE* sock;
    int cid;

    rb_scan_args(argc, argv, "21", &host, &serv, &user);

    Check_Type(host, T_STRING);
    Check_Type(serv, T_STRING);

    sock = register_(newobject_(INT2FIX(0), user));

    if (-1 == (cid = augrt_tcpconnect(StringValueCStr(host),
                                      StringValueCStr(serv), sock))) {
        unregister_(sock);
        rb_raise(cerror_, augrt_error());
    }

    rb_iv_set(*sock, "@id", INT2FIX(cid));
    return *sock;
}

static VALUE
tcplisten_(int argc, VALUE* argv)
{
    VALUE host, serv, user;
    VALUE* sock;
    int cid;

    rb_scan_args(argc, argv, "21", &host, &serv, &user);

    Check_Type(host, T_STRING);
    Check_Type(serv, T_STRING);

    sock = register_(newobject_(INT2FIX(0), user));

    if (-1 == (cid = augrt_tcplisten(StringValueCStr(host),
                                     StringValueCStr(serv), sock))) {
        unregister_(sock);
        rb_raise(cerror_, augrt_error());
    }

    rb_iv_set(*sock, "@id", INT2FIX(cid));
    return *sock;
}

static VALUE
send_(VALUE sock, VALUE buf)
{
    struct augrt_var var = { NULL, NULL };
    VALUE* str;
    int cid = checkid_(sock);

    var.type_ = &vartype_;
    var.arg_ = register_(StringValue(buf));

    if (-1 == augrt_sendv(cid, &var)) {
        unregister_(var.arg_);
        rb_raise(cerror_, augrt_error());
    }

    return Qnil;
}

static VALUE
setrwtimer_(VALUE sock, VALUE ms, VALUE flags)
{
    int cid = checkid_(sock);

    if (-1 == augrt_setrwtimer(cid, NUM2UINT(ms), NUM2UINT(flags)))
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static VALUE
resetrwtimer_(VALUE sock, VALUE ms, VALUE flags)
{
    int cid = checkid_(sock);

    switch (augrt_setrwtimer(cid, NUM2UINT(ms), NUM2UINT(flags))) {
    case -1:
        rb_raise(cerror_, augrt_error());
    case AUGRT_NONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
cancelrwtimer_(VALUE sock, VALUE flags)
{
    int cid = checkid_(sock);

    switch (augrt_cancelrwtimer(cid, NUM2UINT(flags))) {
    case -1:
        rb_raise(cerror_, augrt_error());
    case AUGRT_NONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
settimer_(int argc, VALUE* argv)
{
    VALUE ms, user;
    VALUE* timer;

    unsigned ui;
    struct augrt_var var;
    int tid;

    rb_scan_args(argc, argv, "11", &ms, &user);

    ui = NUM2UINT(ms);
    timer = register_(newobject_(INT2FIX(0), user));

    var.type_ = &vartype_;
    var.arg_ = timer;

    if (-1 == (tid = augrt_settimer(ui, &var))) {
        unregister_(timer);
        rb_raise(cerror_, augrt_error());
    }

    rb_iv_set(*timer, "@id", INT2FIX(tid));
    return *timer;
}

static VALUE
resettimer_(VALUE timer, VALUE ms)
{
    int tid = checkid_(timer);

    switch (augrt_resettimer(tid, NUM2UINT(ms))) {
    case -1:
        rb_raise(cerror_, augrt_error());
    case AUGRT_NONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
canceltimer_(VALUE timer)
{
    int tid = checkid_(timer);

    switch (augrt_canceltimer(tid)) {
    case -1:
        rb_raise(cerror_, augrt_error());
    case AUGRT_NONE:
        return Qfalse;
    }

    return Qtrue;
}

static VALUE
setsslclient_(VALUE sock, VALUE ctx)
{
    int cid = checkid_(sock);
    if (-1 == augrt_setsslclient(cid, StringValueCStr(ctx)))
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static VALUE
setsslserver_(VALUE sock, VALUE ctx)
{
    int cid = checkid_(sock);
    if (-1 == augrt_setsslserver(cid, StringValueCStr(ctx)))
        rb_raise(cerror_, augrt_error());

    return Qnil;
}

static ID id_;

static VALUE
funcall_(VALUE args)
{
    struct session_* session = augrt_getsession()->user_;
    return Qnil == args
        ? rb_funcall(session->module_, id_, 0)
        : rb_apply(session->module_, id_, args);
}

static VALUE
protect_(ID id, VALUE args, int* except)
{
    id_ = id;
    return rb_protect(funcall_, args, except);
}

static VALUE
protect0_(ID id, int* except)
{
    return protect_(id, Qnil, except);
}

static VALUE
protect1_(ID id, VALUE arg1, int* except)
{
    VALUE args = rb_ary_new3(1, arg1);
    return protect_(id, args, except);
}

static VALUE
protect2_(ID id, VALUE arg1, VALUE arg2, int* except)
{
    VALUE args = rb_ary_new3(2, arg1, arg2);
    return protect_(id, args, except);
}

static VALUE
protect3_(ID id, VALUE arg1, VALUE arg2, VALUE arg3, int* except)
{
    VALUE args = rb_ary_new3(3, arg1, arg2, arg3);
    return protect_(id, args, except);
}

static VALUE
initrb_(VALUE nil)
{
    ruby_script("augrb");
    ruby_init_loadpath();

    /* Ids of session functions. */

    stopid_ = rb_intern("stop");
    startid_ = rb_intern("start");
    reconfid_ = rb_intern("reconf");
    eventid_= rb_intern("event");
    closedid_= rb_intern("closed");
    teardownid_= rb_intern("teardown");
    acceptedid_= rb_intern("accepted");
    connectedid_= rb_intern("connected");
    dataid_= rb_intern("data");
    rdexpireid_= rb_intern("rdexpire");
    wrexpireid_= rb_intern("wrexpire");
    expireid_= rb_intern("expire");
    authcertid_= rb_intern("authcert");

    maugrt_ = rb_define_module("Augrt");
    cobject_ = rb_define_class_under(maugrt_, "Object", rb_cObject);
    cerror_ = rb_define_class_under(maugrt_, "Error", rb_eStandardError);

    /* Logger constants. */

    rb_define_const(maugrt_, "LOGCRIT", INT2FIX(AUGRT_LOGCRIT));
    rb_define_const(maugrt_, "LOGERROR", INT2FIX(AUGRT_LOGERROR));
    rb_define_const(maugrt_, "LOGWARN", INT2FIX(AUGRT_LOGWARN));
    rb_define_const(maugrt_, "LOGNOTICE", INT2FIX(AUGRT_LOGNOTICE));
    rb_define_const(maugrt_, "LOGINFO", INT2FIX(AUGRT_LOGINFO));
    rb_define_const(maugrt_, "LOGDEBUG", INT2FIX(AUGRT_LOGDEBUG));

    /* Timer constants. */

    rb_define_const(maugrt_, "TIMRD", INT2FIX(AUGRT_TIMRD));
    rb_define_const(maugrt_, "TIMWR", INT2FIX(AUGRT_TIMWR));
    rb_define_const(maugrt_, "TIMBOTH", INT2FIX(AUGRT_TIMBOTH));

    /* Object methods. */

    rb_define_method(cobject_, "initialize", initobject_, 2);
    rb_define_method(cobject_, "id", objectid_, 0);
    rb_define_method(cobject_, "user", objectuser_, 0);
    rb_define_method(cobject_, "user=", setobjectuser_, 1);

    rb_define_method(cobject_, "<=>", cmpobject_, 1);
    rb_define_method(cobject_, "hash", objecthash_, 0);
    rb_define_method(cobject_, "to_s", objectstr_, 0);

    /* Host module functions. */

    rb_define_module_function(maugrt_, "writelog", writelog_, 2);
    rb_define_module_function(maugrt_, "reconfall", reconfall_, 0);
    rb_define_module_function(maugrt_, "stopall", stopall_, 0);
    rb_define_module_function(maugrt_, "post", post_, -1);
    rb_define_module_function(maugrt_, "dispatch", dispatch_, -1);
    rb_define_module_function(maugrt_, "getenv", getenv_, -1);
    rb_define_module_function(maugrt_, "getsession", getsession_, 0);
    rb_define_module_function(maugrt_, "shutdown", shutdown_, 1);
    rb_define_module_function(maugrt_, "tcpconnect", tcpconnect_, -1);
    rb_define_module_function(maugrt_, "tcplisten", tcplisten_, -1);
    rb_define_module_function(maugrt_, "send", send_, 2);
    rb_define_module_function(maugrt_, "setrwtimer", setrwtimer_, 3);
    rb_define_module_function(maugrt_, "resetrwtimer", resetrwtimer_, 3);
    rb_define_module_function(maugrt_, "cancelrwtimer", cancelrwtimer_, 2);
    rb_define_module_function(maugrt_, "settimer", settimer_, -1);
    rb_define_module_function(maugrt_, "resettimer", resettimer_, 2);
    rb_define_module_function(maugrt_, "canceltimer", canceltimer_, 1);
    rb_define_module_function(maugrt_, "setsslclient", setsslclient_, 2);
    rb_define_module_function(maugrt_, "setsslserver", setsslserver_, 2);

    return Qnil;
}

static void
stop_(void)
{
    struct session_* session = augrt_getsession()->user_;
    assert(session);

    augrt_writelog(AUGRT_LOGINFO, "stop_()");
    if (session->open_ && session->stop_) {
        int except = 0;
        protect0_(stopid_, &except);
    }

    destroysession_(session);
}

static int
start_(struct augrt_session* session)
{
    struct session_* local;
    if (!(local = createsession_(session->name_)))
        return -1;

    session->user_ = local;

    augrt_writelog(AUGRT_LOGINFO, "start_()");
    if (local->start_) {
        int except = 0;
        protect1_(startid_, rb_str_new2(session->name_), &except);
        if (except) {
            destroysession_(local);
            return -1;
        }
    }

    local->open_ = 1;
    return 0;
}

static void
reconf_(void)
{
    struct session_* session = augrt_getsession()->user_;
    assert(session);

    augrt_writelog(AUGRT_LOGINFO, "reconf_()");
    if (session->reconf_) {
        int except = 0;
        protect0_(reconfid_, &except);
    }
}

static void
event_(const char* from, const char* type, const void* user, size_t size)
{
    struct session_* session = augrt_getsession()->user_;
    assert(session);

    augrt_writelog(AUGRT_LOGINFO, "event_()");
    if (session->event_) {
        int except = 0;
        protect3_(eventid_, rb_str_new2(from), rb_str_new2(type),
                  user ? rb_str_new(user, size) : Qnil, &except);
    }
}

static VALUE
eventcall_(VALUE args)
{
    struct session_* session = augrt_getsession()->user_;
    rb_apply(session->module_, eventid_, args);
}

static void
closed_(const struct augrt_object* sock)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    /* Placed on stack, preventing collection. */

    unregister_(sock->user_);

    augrt_writelog(AUGRT_LOGINFO, "closed_()");
    if (session->closed_) {
        int except = 0;
        protect1_(closedid_, user, &except);
    }
}

static void
teardown_(const struct augrt_object* sock)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    augrt_writelog(AUGRT_LOGINFO, "teardown_()");
    if (session->teardown_) {
        int except = 0;
        protect1_(teardownid_, user, &except);
    } else
        augrt_shutdown(sock->id_);
}

static int
accepted_(struct augrt_object* sock, const char* addr, unsigned short port)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = newobject_(sock->id_, rb_iv_get(*(VALUE*)sock->user_, "@user"));

    augrt_writelog(AUGRT_LOGINFO, "accepted_()");
    if (session->accepted_) {
        int except = 0;
        if (Qfalse == protect3_(acceptedid_, user, rb_str_new2(addr),
                                INT2FIX(port), &except) || except)
            return -1;
    }

    sock->user_ = register_(user);
    return 0;
}

static void
connected_(struct augrt_object* sock, const char* addr, unsigned short port)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    augrt_writelog(AUGRT_LOGINFO, "connected_()");
    if (session->connected_) {
        int except = 0;
        protect3_(connectedid_, user, rb_str_new2(addr), INT2FIX(port),
                  &except);
    }
}

static void
data_(const struct augrt_object* sock, const void* buf, size_t len)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    augrt_writelog(AUGRT_LOGINFO, "data_()");
    if (session->data_) {
        int except = 0;
        protect2_(dataid_, user, rb_str_new(buf, len), &except);
    }
}

static void
rdexpire_(const struct augrt_object* sock, unsigned* ms)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    augrt_writelog(AUGRT_LOGINFO, "rdexpire_()");
    if (session->rdexpire_) {
        int except = 0;
        VALUE ret = protect2_(rdexpireid_, user, INT2FIX(*ms), &except);
        if (!except && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static void
wrexpire_(const struct augrt_object* sock, unsigned* ms)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    augrt_writelog(AUGRT_LOGINFO, "wrexpire_()");
    if (session->wrexpire_) {
        int except = 0;
        VALUE ret = protect2_(wrexpireid_, user, INT2FIX(*ms), &except);
        if (!except && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static void
expire_(const struct augrt_object* timer, unsigned* ms)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)timer->user_;

    augrt_writelog(AUGRT_LOGINFO, "expire_()");
    if (session->expire_) {
        int except = 0;
        VALUE ret = protect2_(expireid_, user, INT2FIX(*ms), &except);
        if (!except && FIXNUM_P(ret))
            *ms = FIX2UINT(ret);
    }
}

static int
authcert_(const struct augrt_object* sock, const char* subject,
          const char* issuer)
{
    struct session_* session = augrt_getsession()->user_;
    VALUE user;
    assert(session);
    assert(sock->user_);
    user = *(VALUE*)sock->user_;

    augrt_writelog(AUGRT_LOGINFO, "authcert_()");
    if (session->authcert_) {
        int except = 0;
        if (Qfalse == protect3_(authcertid_, user, rb_str_new2(subject),
                                rb_str_new2(issuer), &except) || except)
            return -1;
    }
    return 0;
}

static const struct augrt_module module_ = {
    stop_,
    start_,
    reconf_,
    event_,
    closed_,
    teardown_,
    accepted_,
    connected_,
    data_,
    rdexpire_,
    wrexpire_,
    expire_,
    authcert_
};

static const struct augrt_module*
init_(const char* name)
{
    int except = 0;
    augrt_writelog(AUGRT_LOGINFO, "init_()");
    ruby_init();

    /* Catch any exceptions. */

    rb_protect(initrb_, Qnil, &except);
    if (except) {
        ruby_finalize();
        return NULL;
    }

    return &module_;
}

static void
term_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "term_()");
    ruby_finalize();
}

AUGRT_MODULE(init_, term_)
