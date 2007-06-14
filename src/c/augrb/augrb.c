#include "augrt.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#if defined(_WIN32)
# define HAVE_ISINF 1
# define _MSC_VER 1200
#endif /* _WIN32 */
#include <ruby.h>

#include "augrt.h"
#include "ruby.h"

static VALUE maugrt_, cobject_;

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
    int lhs = FIX2INT(rb_iv_get(self, "@id"));
    int rhs = FIX2INT(rb_iv_get(other, "@id"));
    int ret;

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
foo_(void)
{
    return rb_str_new2("foo");
}

int
initrb_(void)
{
    ruby_init();
    ruby_script("augrb");

    maugrt_ = rb_define_module("Augrt");
    cobject_ = rb_define_class_under(maugrt_, "Object", rb_cObject);

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

    rb_define_module_function(maugrt_, "foo", foo_, 0);

    return 0;
}

void
termrb_(void)
{
    ruby_finalize();
}

static char*
lowercpy_(char* dst, const char* src)
{
    char* it;
    for (it = dst; '\0' != *src; ++it, ++src)
        *it = tolower(*src);
    return dst;
}

/****************************************************************************/

static void
stop_(void)
{
    augrt_writelog(AUGRT_LOGINFO, "stop_()");
}

static int
start_(struct augrt_session* session)
{
    VALUE m;
    char* file = alloca(strlen(session->name_) + 1);
    lowercpy_(file, session->name_);

    augrt_writelog(AUGRT_LOGINFO, "start_()");

    rb_require(file);
    m = rb_const_get(rb_cObject, rb_intern(session->name_));
    rb_funcall(m, rb_intern("start"), 1, rb_str_new2(session->name_));

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
