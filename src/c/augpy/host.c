/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGMOD_BUILD
#include "augpy/host.h"
#include "augsys/defs.h"

AUG_RCSID("$Id$");

#include "augpy/object.h"

#include "augmod.h"

static PyTypeObject* type_ = NULL;

static int
destroy_(void* arg)
{
    Py_DECREF((PyObject*)arg);
    return 0;
}

static const void*
buf_(void* arg, size_t* size)
{
    const void* buf;
    int len;

    if (-1 == PyObject_AsReadBuffer((PyObject*)arg, &buf, &len)) {
        if (size)
            *size = 0;
        return NULL;
    }

    if (size)
        *size = len;
    return buf;
}

static const struct augmod_vartype vartype_ = {
    destroy_,
    buf_
};

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

    augmod_writelog(level, "%s", msg);
    return incret_(Py_None);
}

static PyObject*
reconfall_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":reconfall"))
        return NULL;

    if (-1 == augmod_reconfall()) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
stopall_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":stopall"))
        return NULL;

    if (-1 == augmod_stopall()) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
post_(PyObject* self, PyObject* args)
{
    const char* to, * type;
    PyObject* user = NULL;
    struct augmod_var var = { NULL, NULL };

    if (!PyArg_ParseTuple(args, "ss|O:post", &to, &type, &user))
        return NULL;

    if (user && user != Py_None) {

        if (!PyObject_CheckReadBuffer(user)) {

            PyErr_SetString(PyExc_TypeError,
                            "post() argument 3 must be string or read-only"
                            " buffer");
            return NULL;
        }

        var.type_ = &vartype_;
        var.arg_ = user;
    }

    if (-1 == augmod_post(to, type, &var)) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    }

    if (user)
        Py_INCREF(user);
    return incret_(Py_None);
}

static PyObject*
dispatch_(PyObject* self, PyObject* args)
{
    const char* to, * type, * user = NULL;
    int size = 0;

    if (!PyArg_ParseTuple(args, "ss|z#:dispatch", &to, &type, &user, &size))
        return NULL;

    if (-1 == augmod_dispatch(to, type, user, size)) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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

    if (!(value = augmod_getenv(name, NULL)))
        return incret_(def);

    return Py_BuildValue("s", value);
}

static PyObject*
getsession_(PyObject* self, PyObject* args)
{
    const struct augmod_session* session;

    if (!PyArg_ParseTuple(args, ":getsession"))
        return NULL;

    if (!(session = augmod_getsession()))
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

    if (-1 == augmod_shutdown(augpy_getid(sock), flags)) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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

    if (!(sock = augpy_createobject(type_, 0, user)))
        return NULL;

    if (-1 == (cid = augmod_tcpconnect(host, serv, sock))) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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

    if (!(sock = augpy_createobject(type_, 0, user)))
        return NULL;

    if (-1 == (lid = augmod_tcplisten(host, serv, sock))) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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
    struct augmod_var var = { NULL, NULL };

    if (!PyArg_ParseTuple(args, "O!O:send", type_, &sock, &buf))
        return NULL;

    if (!PyObject_CheckReadBuffer(buf)) {

        PyErr_SetString(PyExc_TypeError,
                        "send() argument 3 must be string or read-only"
                        " buffer");
        return NULL;
    }

    var.type_ = &vartype_;
    var.arg_ = buf;

    if (-1 == augmod_sendv(augpy_getid(sock), &var)) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    }

    Py_INCREF(buf);
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

    if (-1 == augmod_setrwtimer(augpy_getid(sock), ms, flags)) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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

    switch (augmod_resetrwtimer(augpy_getid(sock), ms, flags)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    case AUGMOD_NONE:
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

    switch (augmod_cancelrwtimer(augpy_getid(sock), flags)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    case AUGMOD_NONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
settimer_(PyObject* self, PyObject* args)
{
    unsigned ms;
    PyObject* user = NULL, * timer;
    struct augmod_var var;
    int tid;

    if (!PyArg_ParseTuple(args, "I|O:settimer", &ms, &user))
        return NULL;

    if (!(timer = augpy_createobject(type_, 0, user)))
        return NULL;

    var.type_ = &vartype_;
    var.arg_ = timer;

    if (-1 == (tid = augmod_settimer(ms, &var))) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        Py_DECREF(timer);
        return NULL;
    }

    augpy_setid(timer, tid);
    return incret_(timer);
}

static PyObject*
resettimer_(PyObject* self, PyObject* args)
{
    PyObject* timer;
    unsigned ms;

    if (!PyArg_ParseTuple(args, "O!I:resettimer", type_, &timer, &ms))
        return NULL;

    switch (augmod_resettimer(augpy_getid(timer), ms)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    case AUGMOD_NONE:
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

    switch (augmod_canceltimer(augpy_getid(timer))) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
        return NULL;
    case AUGMOD_NONE:
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

    if (-1 == augmod_setsslclient(augpy_getid(sock), ctx)) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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

    if (-1 == augmod_setsslserver(augpy_getid(sock), ctx)) {
        PyErr_SetString(PyExc_RuntimeError, augmod_error());
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

    PyModule_AddObject(host, "Object", (PyObject*)type_);

    PyModule_AddIntConstant(host, "LOGCRIT", AUGMOD_LOGCRIT);
    PyModule_AddIntConstant(host, "LOGERROR", AUGMOD_LOGERROR);
    PyModule_AddIntConstant(host, "LOGWARN", AUGMOD_LOGWARN);
    PyModule_AddIntConstant(host, "LOGNOTICE", AUGMOD_LOGNOTICE);
    PyModule_AddIntConstant(host, "LOGINFO", AUGMOD_LOGINFO);
    PyModule_AddIntConstant(host, "LOGDEBUG", AUGMOD_LOGDEBUG);

    PyModule_AddIntConstant(host, "TIMRD", AUGMOD_TIMRD);
    PyModule_AddIntConstant(host, "TIMWR", AUGMOD_TIMWR);
    PyModule_AddIntConstant(host, "TIMRDWR", AUGMOD_TIMRDWR);

    PyModule_AddIntConstant(host, "SHUTNOW", AUGMOD_SHUTNOW);

    return host;
}
