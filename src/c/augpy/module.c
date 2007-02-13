/* Copyright (c) 2004-2007, Mark Aylett <mark@emantic.co.uk>
   See the file COPYING for copying permission.
*/
#define AUGPY_BUILD
#include "augpy/module.h"

#include "augpy/object.h"

#include <augas.h>

static const struct augas_host* host_ = NULL;
static PyTypeObject* type_ = NULL;

static void
destroy_(void* user)
{
    Py_DECREF((PyObject*)user);
}

static PyObject*
incret_(PyObject* x)
{
    Py_INCREF(x);
    return x;
}

static struct augas_event*
setevent_(struct augas_event* event, const char* type, const char* user,
          int size)
{
    strncpy(event->type_, type, sizeof(event->type_));
    event->type_[AUGAS_MAXNAME] ='\0';

    if (user) {
        event->user_ = (void*)user;
        event->size_ = (size_t)size;
    } else {
        event->user_ = NULL;
        event->size_ = 0;
    }
    return event;
}

static PyObject*
writelog_(PyObject* self, PyObject* args)
{
    int level;
    const char* msg;

    if (!PyArg_ParseTuple(args, "is:writelog", &level, &msg))
        return NULL;

    host_->writelog_(level, "%s", msg);
    return incret_(Py_None);
}

static PyObject*
reconf_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":reconf"))
        return NULL;

    host_->reconf_();
    return incret_(Py_None);
}

static PyObject*
stop_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":stop"))
        return NULL;

    host_->stop_();
    return incret_(Py_None);
}

static PyObject*
post_(PyObject* self, PyObject* args)
{
    const char* sname, * to, * ename, * user;
    int size;
    struct augas_event event;

    if (!PyArg_ParseTuple(args, "sssz#:post", &sname, &to, &ename, &user,
                          &size))
        return NULL;

    setevent_(&event, ename, user ? user : strdup(user), size);

    if (-1 == host_->post_(sname, to, &event, event.user_ ? free : NULL)) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        if (event.user_)
            free(event.user_);
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
invoke_(PyObject* self, PyObject* args)
{
    const char* sname, * to, * ename, * user;
    int size;
    struct augas_event event;

    if (!PyArg_ParseTuple(args, "sssz#:invoke", &sname, &to, &ename, &user,
                          &size))
        return NULL;

    setevent_(&event, ename, user, size);

    if (-1 == host_->invoke_(sname, to, &event)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
getenv_(PyObject* self, PyObject* args)
{
    const char* name, * value;

    if (!PyArg_ParseTuple(args, "s:getenv", &name))
        return NULL;

    if (!(value = host_->getenv_(name)))
        return incret_(Py_None);

    return Py_BuildValue("s", value);
}

static PyObject*
shutdown_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    if (!PyArg_ParseTuple(args, "O!:shutdown", type_, &sock))
        return NULL;

    if (-1 == host_->shutdown_(augpy_getid(sock))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
tcpconnect_(PyObject* self, PyObject* args)
{
    const char* sname, * host, * serv;
    PyObject* user, * sock;
    int cid;

    if (!PyArg_ParseTuple(args, "sssO:tcpconnect", &sname, &host, &serv,
                          &user))
        return NULL;

    if (!(sock = augpy_createobject(type_, sname, 0, user)))
        return NULL;

    if (-1 == (cid = host_->tcpconnect_(sname, host, serv, sock))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        Py_DECREF(sock);
        return NULL;
    }

    augpy_setid(sock, cid);
    return incret_(sock);
}

static PyObject*
tcplisten_(PyObject* self, PyObject* args)
{
    const char* sname, * host, * serv;
    PyObject* user, * sock;
    int lid;

    if (!PyArg_ParseTuple(args, "sssO:tcplisten", &sname, &host, &serv,
                          &user))
        return NULL;

    if (!(sock = augpy_createobject(type_, sname, 0, user)))
        return NULL;

    if (-1 == (lid = host_->tcplisten_(sname, host, serv, sock))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        Py_DECREF(sock);
        return NULL;
    }

    augpy_setid(sock, lid);
    return incret_(sock);
}

static PyObject*
send_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    const char* buf;
    int size;

    if (!PyArg_ParseTuple(args, "O!s#:send", type_, &sock, &buf, &size))
        return NULL;

    if (-1 == host_->send_(augpy_getid(sock), buf, size)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
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

    if (-1 == host_->setrwtimer_(augpy_getid(sock), ms, flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
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

    if (-1 == host_->resetrwtimer_(augpy_getid(sock), ms, flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
cancelrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned flags;
    if (!PyArg_ParseTuple(args, "O!I:cancelrwtimer",
                          type_, &sock, &flags))
        return NULL;

    if (-1 == host_->cancelrwtimer_(augpy_getid(sock), flags)) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
settimer_(PyObject* self, PyObject* args)
{
    const char* sname;
    unsigned ms;
    PyObject* user, * timer;
    int tid;

    if (!PyArg_ParseTuple(args, "sIO:settimer", &sname, &ms, &user))
        return NULL;

    if (!(timer = augpy_createobject(type_, sname, 0, user)))
        return NULL;

    if (-1 == (tid = host_->settimer_(sname, ms, timer, destroy_))) {
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
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

    switch (host_->resettimer_(augpy_getid(timer), ms)) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    case AUGAS_NONE:
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

    switch (host_->canceltimer_(augpy_getid(timer))) {
    case -1:
        PyErr_SetString(PyExc_RuntimeError, host_->error_());
        return NULL;
    case AUGAS_NONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyMethodDef methods_[] = {
    {
        "writelog", writelog_, METH_VARARGS,
        "TODO"
    },
    {
        "reconf", reconf_, METH_VARARGS,
        "TODO"
    },
    {
        "stop", stop_, METH_VARARGS,
        "TODO"
    },
    {
        "post", post_, METH_VARARGS,
        "TODO"
    },
    {
        "invoke", invoke_, METH_VARARGS,
        "TODO"
    },
    {
        "getenv", getenv_, METH_VARARGS,
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
    { NULL }
};

PyObject*
augpy_createmodule(const struct augas_host* host, PyTypeObject* type)
{
    PyObject* module = Py_InitModule("augas", methods_);
    if (!module)
        return NULL;

    host_ = host;
    type_ = type;

    PyModule_AddObject(module, "Object", (PyObject*)type_);

    PyModule_AddIntConstant(module, "LOGCRIT", AUGAS_LOGCRIT);
    PyModule_AddIntConstant(module, "LOGERROR", AUGAS_LOGERROR);
    PyModule_AddIntConstant(module, "LOGWARN", AUGAS_LOGWARN);
    PyModule_AddIntConstant(module, "LOGNOTICE", AUGAS_LOGNOTICE);
    PyModule_AddIntConstant(module, "LOGINFO", AUGAS_LOGINFO);
    PyModule_AddIntConstant(module, "LOGDEBUG", AUGAS_LOGDEBUG);

    PyModule_AddIntConstant(module, "TIMRD", AUGAS_TIMRD);
    PyModule_AddIntConstant(module, "TIMWR", AUGAS_TIMWR);
    PyModule_AddIntConstant(module, "TIMBOTH", AUGAS_TIMBOTH);

    return module;
}
