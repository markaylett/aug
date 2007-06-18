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
    return dst;
}

static void
destroysession_(struct session_* session)
{
    free(session);
}

static struct session_*
createsession_(const char* sname)
{
    char* path;
    struct session_* session = malloc(sizeof(struct session_));
    if (!session)
        return NULL;

    path = alloca(strlen(sname) + 1);
    lowercpy_(path, sname);
    rb_require(path);
    session->module_ = rb_const_get(rb_cObject, rb_intern(sname));

    session->stop_
        = rb_const_defined(session->module_, stopid_) ? 1 : 0;
    session->start_
        = rb_const_defined(session->module_, startid_) ? 1 : 0;
    session->reconf_
        = rb_const_defined(session->module_, reconfid_) ? 1 : 0;
    session->event_
        = rb_const_defined(session->module_, eventid_) ? 1 : 0;
    session->closed_
        = rb_const_defined(session->module_, closedid_) ? 1 : 0;
    session->teardown_
        = rb_const_defined(session->module_, teardownid_) ? 1 : 0;
    session->accepted_
        = rb_const_defined(session->module_, acceptedid_) ? 1 : 0;
    session->connected_
        = rb_const_defined(session->module_, connectedid_) ? 1 : 0;
    session->data_
        = rb_const_defined(session->module_, dataid_) ? 1 : 0;
    session->rdexpire_
        = rb_const_defined(session->module_, rdexpireid_) ? 1 : 0;
    session->wrexpire_
        = rb_const_defined(session->module_, wrexpireid_) ? 1 : 0;
    session->expire_
        = rb_const_defined(session->module_, expireid_) ? 1 : 0;
    session->authcert_
        = rb_const_defined(session->module_, authcertid_) ? 1 : 0;

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
    VALUE host, serv;
    VALUE args[] = { INT2FIX(0), Qnil };
    VALUE* sock;
    int cid;

    rb_scan_args(argc, argv, "21", &host, &serv, &argv[1]);

    Check_Type(host, T_STRING);
    Check_Type(serv, T_STRING);

    sock = register_(rb_class_new_instance(2, args, cobject_));

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
    VALUE host, serv;
    VALUE args[] = { INT2FIX(0), Qnil };
    VALUE* sock;
    int cid;

    rb_scan_args(argc, argv, "21", &host, &serv, &argv[1]);

    Check_Type(host, T_STRING);
    Check_Type(serv, T_STRING);

    sock = register_(rb_class_new_instance(2, args, cobject_));

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
    VALUE arg;
    VALUE args[] = { INT2FIX(0), Qnil };
    VALUE* timer;
    unsigned ms;
    int tid;

    rb_scan_args(argc, argv, "11", &ms, &argv[1]);

    ms = NUM2UINT(arg);
    timer = register_(rb_class_new_instance(2, args, cobject_));

    if (-1 == (tid = augrt_settimer(ms, timer))) {
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
    return Qnil;
}

static VALUE
canceltimer_(VALUE timer)
{
    int tid = checkid_(timer);
    return Qnil;
}

static VALUE
setsslclient_(VALUE sock, VALUE ctx)
{
    int cid = checkid_(sock);
    return Qnil;
}

static VALUE
setsslserver_(VALUE sock, VALUE ctx)
{
    int cid = checkid_(sock);
    return Qnil;
}

int
initrb_(void)
{
    ruby_init();
    ruby_script("augrb");

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

    rb_define_const(maugrt_, "LOGCRIT", INT2FIX(AUGRT_LOGCRIT));
    rb_define_const(maugrt_, "LOGERROR", INT2FIX(AUGRT_LOGERROR));
    rb_define_const(maugrt_, "LOGWARN", INT2FIX(AUGRT_LOGWARN));
    rb_define_const(maugrt_, "LOGNOTICE", INT2FIX(AUGRT_LOGNOTICE));
    rb_define_const(maugrt_, "LOGINFO", INT2FIX(AUGRT_LOGINFO));
    rb_define_const(maugrt_, "LOGDEBUG", INT2FIX(AUGRT_LOGDEBUG));

    rb_define_const(maugrt_, "TIMRD", INT2FIX(AUGRT_TIMRD));
    rb_define_const(maugrt_, "TIMWR", INT2FIX(AUGRT_TIMWR));
    rb_define_const(maugrt_, "TIMBOTH", INT2FIX(AUGRT_TIMBOTH));

    rb_define_method(cobject_, "initialize", initobject_, 2);
    rb_define_method(cobject_, "id", objectid_, 0);
    rb_define_method(cobject_, "user", objectuser_, 0);
    rb_define_method(cobject_, "user=", setobjectuser_, 1);

    rb_define_method(cobject_, "<=>", cmpobject_, 1);
    rb_define_method(cobject_, "hash", objecthash_, 0);
    rb_define_method(cobject_, "to_s", objectstr_, 0);

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
    rb_define_module_function(maugrt_, "settimer", settimer_, 2);
    rb_define_module_function(maugrt_, "resettimer", resettimer_, 2);
    rb_define_module_function(maugrt_, "canceltimer", canceltimer_, 1);
    rb_define_module_function(maugrt_, "setsslclient", setsslclient_, 2);
    rb_define_module_function(maugrt_, "setsslserver", setsslserver_, 2);

    return 0;
}

void
termrb_(void)
{
    ruby_finalize();
}

/****************************************************************************/

static void
stop_(void)
{
    struct session_* session = augrt_getsession()->user_;
    assert(session);

    if (session->open_ && session->stop_)
        rb_funcall(session->module_, stopid_, 0);

    destroysession_(session);
}

static int
start_(struct augrt_session* session)
{
    struct session_* local;
    if (!(local = createsession_(session->name_)))
        return -1;

    session->user_ = local;

    if (local->start_)
        rb_funcall(local->module_, startid_, 1, rb_str_new2(session->name_));

    local->open_ = 1;
    return 0;
}

static void
reconf_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "reconf_()");
}

static void
event_(const char* from, const char* type, const void* user, size_t size)
{
    augrt_writelog(AUGRT_LOGINFO, "event_()");
}

static void
closed_(const struct augrt_object* sock)
{
    augrt_writelog(AUGRT_LOGINFO, "closed_()");
}

static void
teardown_(const struct augrt_object* sock)
{
    augrt_writelog(AUGRT_LOGINFO, "teardown_()");
    augrt_shutdown(sock->id_);
}

static int
accepted_(struct augrt_object* sock, const char* addr, unsigned short port)
{
    augrt_writelog(AUGRT_LOGINFO, "accepted_()");
    return 0;
}

static void
connected_(struct augrt_object* sock, const char* addr, unsigned short port)
{
    augrt_writelog(AUGRT_LOGINFO, "connected_()");
}

static void
data_(const struct augrt_object* sock, const void* buf, size_t len)
{
    augrt_writelog(AUGRT_LOGINFO, "data_()");
    augrt_send(sock->id_, buf, len);
}

static void
rdexpire_(const struct augrt_object* sock, unsigned* ms)
{
    augrt_writelog(AUGRT_LOGINFO, "rdexpire_()");
}

static void
wrexpire_(const struct augrt_object* sock, unsigned* ms)
{
    augrt_writelog(AUGRT_LOGINFO, "wrexpire_()");
}

static void
expire_(const struct augrt_object* timer, unsigned* ms)
{
    augrt_writelog(AUGRT_LOGINFO, "expire_()");
}

static int
authcert_(const struct augrt_object* sock, const char* subject,
          const char* issuer)
{
    augrt_writelog(AUGRT_LOGINFO, "authcert_()");
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
    augrt_writelog(AUGRT_LOGINFO, "init_()");
    if (initrb_() < 0)
        return NULL;

    return &module_;
}

static void
term_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "term_()");
    termrb_();
}

AUGRT_MODULE(init_, term_)
