/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGPY_BUILD
#include "augpy/module.h"

#include "augpy/object.h"

#include "augas.h"

#if defined(_WIN32)
# include <direct.h>
#endif /* _WIN32 */

struct import_ {
    PyObject* module_;
    PyObject* stop_;
    PyObject* start_;
    PyObject* reconf_;
    PyObject* event_;
    PyObject* closed_;
    PyObject* teardown_;
    PyObject* accept_;
    PyObject* connected_;
    PyObject* data_;
    PyObject* rdexpire_;
    PyObject* wrexpire_;
    PyObject* expire_;
    int open_;
};

static PyObject* augas_ = NULL;
static PyTypeObject* type_ = NULL;

static void
setpath_(void)
{
    const char* s;
    PyObject* sys;

    if ((s = augas_getenv("rundir")))
        chdir(s);

    if (!(s = augas_getenv("module.augpy.pythonpath")))
        s = "bin";
    else
        augas_writelog(AUGAS_LOGDEBUG, "module.augpy.pythonpath=[%s]", s);

    chdir(s);

    if ((sys = PyImport_ImportModule("sys"))) {

        PyObject* path = PyObject_GetAttrString(sys, "path");
        if (path) {

            char buf[1024];
            PyObject* dir;

            getcwd(buf, sizeof(buf));

            if ((dir = PyString_FromString(buf))) {

                augas_writelog(AUGAS_LOGDEBUG, "adding to sys.path: %s", buf);

                PyList_Insert(path, 0, dir);

                Py_DECREF(dir);
            }

            Py_DECREF(path);
        }

        Py_DECREF(sys);
    }
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

        augas_writelog(AUGAS_LOGERROR, "%s", PyString_AsString(message));

        Py_DECREF(message);
        Py_DECREF(empty);
        Py_DECREF(list);

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
        augas_writelog(AUGAS_LOGDEBUG, "no binding for %s()", name);
        PyErr_Clear();
    }
    return x;
}

static void
destroyimport_(struct import_* import)
{
    if (import->open_ && import->stop_) {

        PyObject* x = PyObject_CallFunction(import->stop_, NULL);
        if (x) {
            Py_DECREF(x);
        } else
            printerr_();
    }

    Py_XDECREF(import->expire_);
    Py_XDECREF(import->wrexpire_);
    Py_XDECREF(import->rdexpire_);
    Py_XDECREF(import->data_);
    Py_XDECREF(import->connected_);
    Py_XDECREF(import->accept_);
    Py_XDECREF(import->teardown_);
    Py_XDECREF(import->closed_);
    Py_XDECREF(import->event_);
    Py_XDECREF(import->reconf_);
    Py_XDECREF(import->start_);
    Py_XDECREF(import->stop_);

    Py_XDECREF(import->module_);
    free(import);
}

static struct import_*
createimport_(const char* sname)
{
    struct import_* import = calloc(1, sizeof(struct import_));
    if (!import)
        return NULL;

    import->open_ = 0;
    if (!(import->module_ = PyImport_ImportModule((char*)sname))) {
        printerr_();
        goto fail;
    }

    import->stop_ = getmethod_(import->module_, "stop");
    import->start_ = getmethod_(import->module_, "start");
    import->reconf_ = getmethod_(import->module_, "reconf");
    import->event_ = getmethod_(import->module_, "event");
    import->closed_ = getmethod_(import->module_, "closed");
    import->teardown_ = getmethod_(import->module_, "teardown");
    import->accept_ = getmethod_(import->module_, "accept");
    import->connected_ = getmethod_(import->module_, "connected");
    import->data_ = getmethod_(import->module_, "data");
    import->rdexpire_ = getmethod_(import->module_, "rdexpire");
    import->wrexpire_ = getmethod_(import->module_, "wrexpire");
    import->expire_ = getmethod_(import->module_, "expire");

    return import;

 fail:
    destroyimport_(import);
    return NULL;
}

static void
termpy_(void)
{
    int level, objects;

    if (!Py_IsInitialized())
        return;

    augas_writelog(AUGAS_LOGDEBUG, "finalising python interpreter");
    Py_Finalize();

    objects = augpy_objects();
    level = objects ? AUGAS_LOGERROR : AUGAS_LOGINFO;
    augas_writelog(level, "allocated objects: %d", objects);
}

static int
initpy_(void)
{
    augas_writelog(AUGAS_LOGDEBUG, "initialising python interpreter");
    Py_Initialize();
    /* Py_VerboseFlag = 1; */
    setpath_();

    augas_writelog(AUGAS_LOGDEBUG, "initialising augas module");
    if (!(type_ = augpy_createtype()))
        goto fail;

    if (!(augas_ = augpy_createmodule(type_)))
        goto fail;
    return 0;

 fail:
    termpy_();
    return -1;
}

static void
stop_(void)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    destroyimport_(import);
}

static int
start_(struct augas_serv* serv)
{
    struct import_* import;
    if (!(import = createimport_(serv->name_)))
        return -1;

    serv->user_ = import;

    if (import->start_) {

        PyObject* x = PyObject_CallFunction(import->start_, "s", serv->name_);
        if (!x) {
            printerr_();
            destroyimport_(import);
            return -1;
        }
        Py_DECREF(x);
    }

    import->open_ = 1;
    return 0;
}

static void
reconf_(void)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);

    if (import->reconf_) {

        PyObject* x = PyObject_CallFunction(import->reconf_, NULL);
        if (x) {
            Py_DECREF(x);
        } else
            printerr_();
    }
}

static void
event_(const char* from, const struct augas_event* event)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);

    if (import->event_) {

        PyObject* y = PyObject_CallFunction(import->event_, "ssz#",
                                            from, event->type_,
                                            (const char*)event->user_,
                                            (int)event->size_);
        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }

    /* x will be Py_DECREF()-ed by destroy_(). */
}

static void
closed_(const struct augas_object* sock)
{
    struct import_* import = augas_getserv()->user_;
    PyObject* x = sock->user_;
    assert(import);
    assert(x);

    if (import->closed_) {

        PyObject* y = PyObject_CallFunction(import->closed_, "O", x);
        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }

    Py_DECREF(x);
}

static void
teardown_(const struct augas_object* sock)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    assert(sock->user_);

    if (import->teardown_) {

        PyObject* x = sock->user_;
        PyObject* y = PyObject_CallFunction(import->teardown_, "O", x);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();

    } else
        augas_shutdown(sock->id_);
}

static int
accept_(struct augas_object* sock, const char* addr, unsigned short port)
{
    struct import_* import = augas_getserv()->user_;
    PyObject* x, * y;
    int ret = 0;
    assert(import);
    assert(sock->user_);

    x = augpy_getuser(sock->user_);
    y = augpy_createobject(type_, sock->id_, x);
    Py_DECREF(x);

    if (!y) {

        /* closed() will not be called if accept() fails. */

        printerr_();
        ret = -1;

    } else if (import->accept_) {

        PyObject* z = PyObject_CallFunction(import->accept_, "OsH",
                                            y, addr, port);

        if (!z) {

            /* closed() will not be called if accept() fails. */

            printerr_();
            Py_DECREF(y);
            return -1;
        }

        if (z == Py_False) {

            augas_writelog(AUGAS_LOGDEBUG, "accept() handler returned false");

            /* closed() will not be called if accept() fails. */

            Py_DECREF(y);
            y = NULL;
            ret = -1;
        }

        Py_DECREF(z);
    }

    /* The original user data is still retained by the listener. */

    sock->user_ = y;
    return ret;
}

static void
connected_(struct augas_object* sock, const char* addr, unsigned short port)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    assert(sock->user_);

    if (import->connected_) {

        PyObject* x = sock->user_;
        PyObject* y = PyObject_CallFunction(import->connected_, "OsH",
                                            x, addr, port);

        if (y) {
            Py_DECREF(y);
        } else
            printerr_();
    }

    /* closed() will always be called, even if connected() fails. */
}

static void
data_(const struct augas_object* sock, const char* buf, size_t size)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    assert(sock->user_);

    if (import->data_) {

        PyObject* x = sock->user_;
        PyObject* y = PyBuffer_FromMemory((void*)buf, (int)size);
        PyObject* z = PyObject_CallFunction(import->data_, "OO", x, y);

        if (z) {
            Py_DECREF(z);
        } else
            printerr_();

        Py_DECREF(y);
    }
}

static void
rdexpire_(const struct augas_object* sock, unsigned* ms)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    assert(sock->user_);

    if (import->rdexpire_) {

        PyObject* x = sock->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->rdexpire_, "OO", x, y);

        if (z) {
            if (PyInt_Check(z)) {
                augas_writelog(AUGAS_LOGDEBUG,
                               "handler returned new timeout value");
                *ms = PyInt_AsLong(z);
            }
            Py_DECREF(z);
        } else
            printerr_();

        Py_DECREF(y);
    }
}

static void
wrexpire_(const struct augas_object* sock, unsigned* ms)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    assert(sock->user_);

    if (import->wrexpire_) {

        PyObject* x = sock->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->wrexpire_, "OO", x, y);

        if (z) {
            if (PyInt_Check(z)) {
                augas_writelog(AUGAS_LOGDEBUG,
                               "handler returned new timeout value");
                *ms = PyInt_AsLong(z);
            }
            Py_DECREF(z);
        } else
            printerr_();

        Py_DECREF(y);
    }
}

static void
expire_(const struct augas_object* timer, unsigned* ms)
{
    struct import_* import = augas_getserv()->user_;
    assert(import);
    assert(timer->user_);

    if (import->expire_) {

        PyObject* x = timer->user_;
        PyObject* y = PyInt_FromLong(*ms);
        PyObject* z = PyObject_CallFunction(import->expire_, "OO", x, y);
        Py_DECREF(y);

        if (z) {
            if (PyInt_Check(z)) {
                augas_writelog(AUGAS_LOGDEBUG,
                               "handler returned new timeout value");
                *ms = PyInt_AsLong(z);
            }
            Py_DECREF(z);
        } else
            printerr_();

        /* x will be Py_DECREF()-ed by destroy_() when *ms == 0. */
    }
}

static const struct augas_module module_ = {
    stop_,
    start_,
    reconf_,
    event_,
    closed_,
    teardown_,
    accept_,
    connected_,
    data_,
    rdexpire_,
    wrexpire_,
    expire_
};

static const struct augas_module*
init_(const char* name)
{
    augas_writelog(AUGAS_LOGINFO, "initialising augpy module");

    if (initpy_() < 0)
        return NULL;

    return &module_;
}

static void
term_(void)
{
    augas_writelog(AUGAS_LOGINFO, "terminating augpy module");
    termpy_();
}

AUGAS_MODULE(init_, term_)
