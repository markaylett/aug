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

#include "host.h"
#include "object.h"

#include "augses.h"

#if defined(_WIN32)
# include <direct.h>
#endif /* _WIN32 */

struct import_ {
    PyObject* module_;
    PyObject* start_;
    PyObject* stop_;
    PyObject* reconf_;
    PyObject* event_;
    PyObject* closed_;
    PyObject* teardown_;
    PyObject* accepted_;
    PyObject* connected_;
    PyObject* auth_;
    PyObject* recv_;
    PyObject* mrecv_;
    PyObject* error_;
    PyObject* rdexpire_;
    PyObject* wrexpire_;
    PyObject* expire_;
    mod_bool open_;
};

static PyObject* augpy_ = NULL;
static PyTypeObject* type_ = NULL;

static void
setpath_(void)
{
    const char* s;
    char prev[1024];
    PyObject* sys;

    /* Store working directory for later restoration. */

    getcwd(prev, sizeof(prev));

    /* Path may be relative to run directory. */

    if ((s = mod_getenv("rundir", NULL)))
        chdir(s);

    s = mod_getenv("module.augpy.pythonpath", "python");
    mod_writelog(MOD_LOGDEBUG, "module.augpy.pythonpath=[%s]", s);
    chdir(os);

    if ((sys = PyImport_ImportModule("sys"))) {

        PyObject* path = PyObject_GetAttrString(sys, "path");
        if (path) {

            char buf[1024];
            PyObject* dir;

            getcwd(buf, sizeof(buf));

            if ((dir = PyString_FromString(buf))) {

                mod_writelog(MOD_LOGDEBUG, "adding to sys.path: %s", buf);

                PyList_Append(path, dir);

                Py_DECREF(dir);
            }

            Py_DECREF(path);
        }

        Py_DECREF(sys);
    }

    /* Restore previous working directory. */

    chdir(prev);
}

static void
printerr_(void)
{
    PyObject* type, * value, * traceback;
    PyObject* module;

    if (!PyErr_Occurred())
        return;

    /* Returns owned references. */

    PyErr_Fetch(&type, &value, &traceback);
    if ((module = PyImport_ImportModule("traceback"))) {

        PyObject* list, * empty, * message;
        list = PyObject_CallMethod(module, "format_exception", "OOO", type,
                                   value == NULL ? Py_None : value,
                                   traceback == NULL ? Py_None : traceback);
        empty = PyString_FromString("");
        message = PyObject_CallMethod(empty, "join", "O", list);

        mod_writelog(MOD_LOGERROR, "%s", PyString_AsString(message));

        Py_DECREF(list);
        Py_DECREF(empty);
        Py_DECREF(message);

        Py_DECREF(module);

    } else
        PyErr_Print();

    Py_DECREF(type);
    Py_XDECREF(value);
    Py_XDECREF(traceback);

    PyErr_Clear();
}

static PyObject*
getmethod_(PyObject* module, const char* name)
{
    PyObject* x = PyObject_GetAttrString(module, (char*)name);
    if (x) {
        if (!PyCallable_Check(x)) {
            Py_DECREF(x);
            x = NULL;
        }
    } else {
        mod_writelog(MOD_LOGDEBUG, "no binding for %s()", name);
        PyErr_Clear();
    }
    return x;
}

static void
termimport_(struct import_* import)
{
    Py_XDECREF(import->expire_);
    Py_XDECREF(import->wrexpire_);
    Py_XDECREF(import->rdexpire_);
    Py_XDECREF(import->error_);
    Py_XDECREF(import->recv_);
    Py_XDECREF(import->mrecv_);
    Py_XDECREF(import->auth_);
    Py_XDECREF(import->connected_);
    Py_XDECREF(import->accepted_);
    Py_XDECREF(import->teardown_);
    Py_XDECREF(import->closed_);
    Py_XDECREF(import->event_);
    Py_XDECREF(import->reconf_);
    Py_XDECREF(import->stop_);
    Py_XDECREF(import->start_);

    Py_XDECREF(import->module_);
}

static mod_bool
initimport_(struct import_* import, const char* sname)
{
    if (!(import->module_ = PyImport_ImportModule((char*)sname))) {
        printerr_();
        return MOD_FALSE;
    }

    import->start_ = getmethod_(import->module_, "start");
    import->stop_ = getmethod_(import->module_, "stop");
    import->reconf_ = getmethod_(import->module_, "reconf");
    import->event_ = getmethod_(import->module_, "event");
    import->closed_ = getmethod_(import->module_, "closed");
    import->teardown_ = getmethod_(import->module_, "teardown");
    import->accepted_ = getmethod_(import->module_, "accepted");
    import->connected_ = getmethod_(import->module_, "connected");
    import->auth_ = getmethod_(import->module_, "auth");
    import->recv_ = getmethod_(import->module_, "recv");
    import->mrecv_ = getmethod_(import->module_, "mrecv");
    import->error_ = getmethod_(import->module_, "error");
    import->rdexpire_ = getmethod_(import->module_, "rdexpire");
    import->wrexpire_ = getmethod_(import->module_, "wrexpire");
    import->expire_ = getmethod_(import->module_, "expire");
    import->open_ = MOD_FALSE;

    return MOD_TRUE;
}

static void
termpy_(void)
{
    int level, objects;

    if (!Py_IsInitialized())
        return;

    mod_writelog(MOD_LOGDEBUG, "finalising python interpreter");
    Py_Finalize();

    objects = augpy_handles();
    level = objects ? MOD_LOGERROR : MOD_LOGINFO;
    mod_writelog(level, "allocated objects: %d", objects);
}

static mod_bool
initpy_(void)
{
    mod_writelog(MOD_LOGDEBUG, "initialising python interpreter");
    Py_Initialize();
    /* Py_VerboseFlag = 1; */
    setpath_();

    mod_writelog(MOD_LOGDEBUG, "initialising aug module");
    if (!(type_ = augpy_createtype()))
        goto fail;

    if (!(augpy_ = augpy_createhost(type_)))
        goto fail;
    return MOD_TRUE;

 fail:
    termpy_();
    return MOD_FALSE;
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

        PyObject* x = PyObject_CallFunction(import->start_, "s", impl->name_);
        if (!x) {

            printerr_();
            return MOD_FALSE;

        } else if (x == Py_False) {

            /* Treat Py_False like any other object with respect to reference
               counts. */

            Py_DECREF(x);
            return MOD_FALSE;
        }
        Py_DECREF(x);
    }

    /* Default. */

    import->open_ = MOD_TRUE;
    return MOD_TRUE;
}

static void
stop_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    if (import->open_ && import->stop_) {

        PyObject* x = PyObject_CallFunction(import->stop_, NULL);
        if (x) {
            Py_DECREF(x);
        } else
            printerr_();
    }
}

static void
reconf_(mod_session* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    if (import->reconf_) {

        PyObject* x = PyObject_CallFunction(import->reconf_, NULL);
        if (x) {
            Py_DECREF(x);
        } else
            printerr_();
    }
}

static void
event_(mod_session* ob_, const char* from, const char* type, mod_id id,
       aug_object* ob)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob_);
    struct import_* import = &impl->import_;

    if (import->event_) {

        PyObject* y;
        if (ob) {

            /* Preference is augpy_boxpy type. */

            PyObject* x = augpy_obtopy(ob);
            if (x) {

                y = PyObject_CallFunction(import->event_, "ssiO", from, type,
                                          id, x);
                Py_DECREF(x);
                goto done;

            } else {

                /* Fallback to aug_blob type. */

                aug_blob* blob = aug_cast(ob, aug_blobid);
                if (blob) {

                    size_t size;
                    const void* data = aug_getblobdata(blob, &size);

                    /* Unsafe to release here. */

                    if (data) {

                        /* Blob data obtained. */

                        y = PyObject_CallFunction(import->event_, "ssiz#",
                                                  from, type, id,
                                                  (const char*)data, size);
                        aug_release(blob);
                        goto done;
                    }
                    aug_release(blob);
                }
            }
        }

        /* Null or unsupported object type. */

        y = PyObject_CallFunction(import->event_, "ssiO", from, type,
                                  id, Py_None);

    done:
        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }
}

static void
closed_(mod_session* ob, struct mod_handle* sock)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->closed_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyObject_CallFunction(import->closed_, "O", x);
        Py_DECREF(x);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }

    aug_assign(sock->ob_, NULL);
}

static void
teardown_(mod_session* ob, struct mod_handle* sock)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->teardown_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyObject_CallFunction(import->teardown_, "O", x);
        Py_DECREF(x);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();

    } else
        mod_shutdown(sock->id_, 0);
}

static mod_bool
accepted_(mod_session* ob, struct mod_handle* sock, const char* name)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    PyObject* x;
    augpy_box* box;
    mod_bool ret = MOD_TRUE;

    assert(sock->ob_);

    x = augpy_obtopy(sock->ob_);
    box = augpy_boxhandle(type_, sock->id_, x);
    Py_DECREF(x);

    if (!box) {

        /* closed() will not be called if accepted() fails. */

        printerr_();
        return MOD_FALSE;
    }

    if (import->accepted_) {

        PyObject* y = box->vtbl_->unbox_(box);
        PyObject* z = PyObject_CallFunction(import->accepted_, "Os", y, name);
        Py_DECREF(y);

        if (!z) {

            /* closed() will not be called if accepted() fails. */

            printerr_();
            ret = MOD_FALSE;
            goto done;
        }

        if (z == Py_False) {

            mod_writelog(MOD_LOGDEBUG,
                         "accepted() handler returned false");

            /* closed() will not be called if accepted() fails. */

            Py_DECREF(z);
            ret = MOD_FALSE;
            goto done;
        }

        Py_DECREF(z);
    }

    /* The original user data is still retained by the listener. */

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

    if (import->connected_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyObject_CallFunction(import->connected_, "Os", x,
                                            name);
        Py_DECREF(x);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }

    /* closed() will always be called, even if connected() fails. */
}

static mod_bool
auth_(mod_session* ob, struct mod_handle* sock, const char* subject,
      const char* issuer)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    mod_bool ret = MOD_TRUE;

    assert(sock->ob_);

    if (import->auth_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyObject_CallFunction(import->auth_, "Oss", x, subject,
                                            issuer);
        Py_DECREF(x);

        if (y) {
            if (y == Py_False)
                ret = MOD_FALSE;
            Py_DECREF(y);
        } else {
            printerr_();
            ret = MOD_FALSE;
        }
    }

    return ret;
}

static void
recv_(mod_session* ob, struct mod_handle* sock, const void* buf, size_t len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->recv_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyBuffer_FromMemory((void*)buf, (int)len);
        PyObject* z = PyObject_CallFunction(import->recv_, "OO", x, y);
        Py_DECREF(x);
        Py_DECREF(y);

        if (z) {
            Py_DECREF(z);
        } else
            printerr_();
    }
}

static void
mrecv_(mod_session* ob, const char* node, unsigned sess, unsigned short type,
       const void* buf, size_t len)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    if (import->mrecv_) {

        PyObject* x, * y;
        if (buf) {
            x = PyBuffer_FromMemory((void*)buf, (int)len);
        } else {
            x = Py_None;
            Py_INCREF(x);
        }

        y = PyObject_CallFunction(import->mrecv_, "sIHO",
                                  node, sess, type, x);
        Py_DECREF(x);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }
}

static void
error_(mod_session* ob, struct mod_handle* sock, const char* desc)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->error_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyObject_CallFunction(import->error_, "Os", x, desc);
        Py_DECREF(x);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }
}

static void
rdexpire_(mod_session* ob, struct mod_handle* sock, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->rdexpire_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->rdexpire_, "OO", x, y);
        Py_DECREF(x);
        Py_DECREF(y);

        if (z) {
            if (PyInt_Check(z)) {
                mod_writelog(MOD_LOGDEBUG,
                             "handler returned new timeout value");
                *ms = PyInt_AsLong(z);
            }
            Py_DECREF(z);
        } else
            printerr_();
    }
}

static void
wrexpire_(mod_session* ob, struct mod_handle* sock, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(sock->ob_);

    if (import->wrexpire_) {

        PyObject* x = augpy_obtopy(sock->ob_);
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->wrexpire_, "OO", x, y);
        Py_DECREF(x);
        Py_DECREF(y);

        if (z) {
            if (PyInt_Check(z)) {
                mod_writelog(MOD_LOGDEBUG,
                             "handler returned new timeout value");
                *ms = PyInt_AsLong(z);
            }
            Py_DECREF(z);
        } else
            printerr_();
    }
}

static void
expire_(mod_session* ob, struct mod_handle* timer, unsigned* ms)
{
    struct impl_* impl = AUG_PODIMPL(struct impl_, session_, ob);
    struct import_* import = &impl->import_;

    assert(timer->ob_);

    if (import->expire_) {

        PyObject* x, * y, * z;
        if (!(x = augpy_obtopy(timer->ob_))) {
            x = Py_None;
            Py_INCREF(x);
        }

        y = PyInt_FromLong(*ms);
        z = PyObject_CallFunction(import->expire_, "OO", x, y);
        Py_DECREF(x);
        Py_DECREF(y);

        if (z) {
            if (PyInt_Check(z)) {
                mod_writelog(MOD_LOGDEBUG,
                             "handler returned new timeout value");
                *ms = PyInt_AsLong(z);
            }
            Py_DECREF(z);
        } else
            printerr_();
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
    mrecv_,
    error_,
    rdexpire_,
    wrexpire_,
    expire_
};

static mod_bool
init_(const char* name)
{
    mod_writelog(MOD_LOGINFO, "initialising augpy module");
    return initpy_();
}

static void
term_(void)
{
    mod_writelog(MOD_LOGINFO, "terminating augpy module");
    termpy_();
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
