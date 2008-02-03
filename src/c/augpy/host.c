/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define MOD_BUILD
#include "augpy/host.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "augpy/object.h"

static PyTypeObject* type_ = NULL;

static PyObject*
incret_(PyObject* x)
{
    Py_INCREF(x);
    return x;
}

static PyObject*
writelog_(PyObject* self, PyObject* args)
{
    int level;
    const char* msg;

    if (!PyArg_ParseTuple(args, "is:writelog", &level, &msg))
        return NULL;

    mod_writelog(level, "%s", msg);
    return incret_(Py_None);
}

static PyObject*
reconfall_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":reconfall"))
        return NULL;

    if (-1 == mod_reconfall()) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
stopall_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":stopall"))
        return NULL;

    if (-1 == mod_stopall()) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
post_(PyObject* self, PyObject* args)
{
    const char* to, * type;
    PyObject* user = NULL;
    aug_blob* blob = NULL;
    int ret;

    if (!PyArg_ParseTuple(args, "ss|O:post", &to, &type, &user))
        return NULL;

    if (user && user != Py_None) {

        if (!PyObject_CheckReadBuffer(user)) {

            PyErr_SetString(PyExc_TypeError,
                            "post() argument 3 must be string or"
                            " read-only buffer");
            return NULL;
        }

        if (!(blob = augpy_createblob(user)))
            return NULL;
    }

    ret = mod_post(to, type, (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (-1 == ret) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
dispatch_(PyObject* self, PyObject* args)
{
    const char* to, * type;
    PyObject* user = NULL;
    aug_blob* blob = NULL;
    int ret;

    if (!PyArg_ParseTuple(args, "ss|O:dispatch", &to, &type, &user))
        return NULL;

    if (user && user != Py_None) {

        if (!PyObject_CheckReadBuffer(user)) {

            PyErr_SetString(PyExc_TypeError,
                            "dispatch() argument 3 must be string or"
                            " read-only buffer");
            return NULL;
        }

        if (!(blob = augpy_createblob(user)))
            return NULL;
    }

    ret = mod_dispatch(to, type, (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (-1 == ret) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
getenv_(PyObject* self, PyObject* args)
{
    const char* name, * value;
    PyObject* def = Py_None;

    if (!PyArg_ParseTuple(args, "s|O:getenv", &name, &def))
        return NULL;

    if (!(value = mod_getenv(name, NULL)))
        return incret_(def);

    return Py_BuildValue("s", value);
}

static PyObject*
getsession_(PyObject* self, PyObject* args)
{
    const struct mod_session* session;

    if (!PyArg_ParseTuple(args, ":getsession"))
        return NULL;

    if (!(session = mod_getsession()))
        return incret_(Py_None);

    return Py_BuildValue("s", session->name_);
}

static PyObject*
shutdown_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned flags;
    if (!PyArg_ParseTuple(args, "O!I:shutdown", type_, &sock, &flags))
        return NULL;

    if (-1 == mod_shutdown(augpy_getid(sock), flags)) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
tcpconnect_(PyObject* self, PyObject* args)
{
    const char* host, * serv;
    PyObject* user = NULL, * sock;
    int cid;

    if (!PyArg_ParseTuple(args, "ss|O:tcpconnect", &host, &serv, &user))
        return NULL;

    if (!(sock = augpy_createhandle(type_, 0, user)))
        return NULL;

    if (-1 == (cid = mod_tcpconnect(host, serv, sock))) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        Py_DECREF(sock);
        return NULL;
    }

    augpy_setid(sock, cid);
    return incret_(sock);
}

static PyObject*
tcplisten_(PyObject* self, PyObject* args)
{
    const char* host, * serv;
    PyObject* user = NULL, * sock;
    int lid;

    if (!PyArg_ParseTuple(args, "ss|O:tcplisten", &host, &serv, &user))
        return NULL;

    if (!(sock = augpy_createhandle(type_, 0, user)))
        return NULL;

    if (-1 == (lid = mod_tcplisten(host, serv, sock))) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        Py_DECREF(sock);
        return NULL;
    }

    augpy_setid(sock, lid);
    return incret_(sock);
}

static PyObject*
send_(PyObject* self, PyObject* args)
{
    PyObject* sock, * buf;
    aug_blob* blob;
    int ret;

    if (!PyArg_ParseTuple(args, "O!O:send", type_, &sock, &buf))
        return NULL;

    if (!PyObject_CheckReadBuffer(buf)) {

        PyErr_SetString(PyExc_TypeError,
                        "send() argument 3 must be string or read-only"
                        " buffer");
        return NULL;
    }

    if (!(blob = augpy_createblob(buf)))
        return NULL;

    /* mod_sendv() takes ownership. */

    ret = mod_sendv(augpy_getid(sock), blob);
    aug_release(blob);

    if (-1 == ret) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
setrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned ms, flags;

    if (!PyArg_ParseTuple(args, "O!II:setrwtimer",
                          type_, &sock, &ms, &flags))
        return NULL;

    if (-1 == mod_setrwtimer(augpy_getid(sock), ms, flags)) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
resetrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned ms, flags;

    if (!PyArg_ParseTuple(args, "O!II:resetrwtimer",
                          type_, &sock, &ms, &flags))
        return NULL;

    switch (mod_resetrwtimer(augpy_getid(sock), ms, flags)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    case MOD_NONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
cancelrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned flags;
    if (!PyArg_ParseTuple(args, "O!I:cancelrwtimer",
                          type_, &sock, &flags))
        return NULL;

    switch (mod_cancelrwtimer(augpy_getid(sock), flags)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    case MOD_NONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
settimer_(PyObject* self, PyObject* args)
{
    unsigned ms;
    PyObject* user = NULL, * timer;
    aug_blob* blob;
    int tid;

    if (!PyArg_ParseTuple(args, "I|O:settimer", &ms, &user))
        return NULL;

    if (!(timer = augpy_createhandle(type_, 0, user)))
        return NULL;

    /* Both blob and this function hold reference to sock. */

    if (!(blob = augpy_createblob(timer))) {
        Py_DECREF(timer);
        return NULL;
    }

    /* mod_settimer() takes ownership. */

    tid = mod_settimer(ms, (aug_object*)blob);
    aug_release(blob);

    if (-1 == tid) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        Py_DECREF(timer);
        return NULL;
    }

    augpy_setid(timer, tid);
    return timer; /* Ref already held; no need to retain_(). */
}

static PyObject*
resettimer_(PyObject* self, PyObject* args)
{
    PyObject* timer;
    unsigned ms;

    if (!PyArg_ParseTuple(args, "O!I:resettimer", type_, &timer, &ms))
        return NULL;

    switch (mod_resettimer(augpy_getid(timer), ms)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    case MOD_NONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
canceltimer_(PyObject* self, PyObject* args)
{
    PyObject* timer;

    if (!PyArg_ParseTuple(args, "O!:canceltimer", type_, &timer))
        return NULL;

    switch (mod_canceltimer(augpy_getid(timer))) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    case MOD_NONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
setsslclient_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    const char* ctx;

    if (!PyArg_ParseTuple(args, "O!s:setsslclient", type_, &sock, &ctx))
        return NULL;

    if (-1 == mod_setsslclient(augpy_getid(sock), ctx)) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
setsslserver_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    const char* ctx;

    if (!PyArg_ParseTuple(args, "O!s:setsslserver", type_, &sock, &ctx))
        return NULL;

    if (-1 == mod_setsslserver(augpy_getid(sock), ctx)) {
        PyErr_SetString(PyExc_RuntimeError, mod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyMethodDef methods_[] = {
    {
        "writelog", writelog_, METH_VARARGS,
        "TODO"
    },
    {
        "reconfall", reconfall_, METH_VARARGS,
        "TODO"
    },
    {
        "stopall", stopall_, METH_VARARGS,
        "TODO"
    },
    {
        "post", post_, METH_VARARGS,
        "TODO"
    },
    {
        "dispatch", dispatch_, METH_VARARGS,
        "TODO"
    },
    {
        "getenv", getenv_, METH_VARARGS,
        "TODO"
    },
    {
        "getsession", getsession_, METH_VARARGS,
        "TODO"
    },
    {
        "shutdown", shutdown_, METH_VARARGS,
        "TODO"
    },
    {
        "tcpconnect", tcpconnect_, METH_VARARGS,
        "TODO"
    },
    {
        "tcplisten", tcplisten_, METH_VARARGS,
        "TODO"
    },
    {
        "send", send_, METH_VARARGS,
        "TODO"
    },
    {
        "setrwtimer", setrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "resetrwtimer", resetrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "cancelrwtimer", cancelrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "settimer", settimer_, METH_VARARGS,
        "TODO"
    },
    {
        "resettimer", resettimer_, METH_VARARGS,
        "TODO"
    },
    {
        "canceltimer", canceltimer_, METH_VARARGS,
        "TODO"
    },
    {
        "setsslclient", setsslclient_, METH_VARARGS,
        "TODO"
    },
    {
        "setsslserver", setsslserver_, METH_VARARGS,
        "TODO"
    },
    { NULL }
};

PyObject*
augpy_createhost(PyTypeObject* type)
{
    PyObject* host = Py_InitModule("augpy", methods_);
    if (!host)
        return NULL;

    type_ = type;

    PyModule_AddObject(host, "Handle", (PyObject*)type_);

    PyModule_AddIntConstant(host, "LOGCRIT", MOD_LOGCRIT);
    PyModule_AddIntConstant(host, "LOGERROR", MOD_LOGERROR);
    PyModule_AddIntConstant(host, "LOGWARN", MOD_LOGWARN);
    PyModule_AddIntConstant(host, "LOGNOTICE", MOD_LOGNOTICE);
    PyModule_AddIntConstant(host, "LOGINFO", MOD_LOGINFO);
    PyModule_AddIntConstant(host, "LOGDEBUG", MOD_LOGDEBUG);

    PyModule_AddIntConstant(host, "TIMRD", MOD_TIMRD);
    PyModule_AddIntConstant(host, "TIMWR", MOD_TIMWR);
    PyModule_AddIntConstant(host, "TIMRDWR", MOD_TIMRDWR);

    PyModule_AddIntConstant(host, "SHUTNOW", MOD_SHUTNOW);

    return host;
}
