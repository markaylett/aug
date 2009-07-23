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
#include "host.h"
#include "augctx/defs.h"

AUG_RCSID("$Id$");

#include "object.h"

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

    if (mod_reconfall() < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
stopall_(PyObject* self, PyObject* args)
{
    if (!PyArg_ParseTuple(args, ":stopall"))
        return NULL;

    if (mod_stopall() < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
post_(PyObject* self, PyObject* args)
{
    const char* to, * type;
    mod_id id;
    PyObject* ob = NULL;
    aug_blob* blob = NULL;
    mod_result result;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "ssi|O:post", &to, &type, &id, &ob))
        return NULL;

    if (ob && ob != Py_None) {

        if (!PyObject_CheckReadBuffer(ob)) {

            PyErr_SetString(PyExc_TypeError,
                            "post() argument 3 must be string or"
                            " read-only buffer");
            return NULL;
        }

        if (!(blob = augpy_createblob(ob)))
            return NULL;
    }

    result = mod_post(to, type, id, (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (result < 0) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
dispatch_(PyObject* self, PyObject* args)
{
    const char* to, * type;
    mod_id id;
    PyObject* ob = NULL;
    aug_blob* blob = NULL;
    mod_result result;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "ssi|O:dispatch", &to, &type, &id, &ob))
        return NULL;

    if (ob && ob != Py_None) {

        if (!PyObject_CheckReadBuffer(ob)) {

            PyErr_SetString(PyExc_TypeError,
                            "dispatch() argument 3 must be string or"
                            " read-only buffer");
            return NULL;
        }

        if (!(blob = augpy_createblob(ob)))
            return NULL;
    }

    result = mod_dispatch(to, type, id, (aug_object*)blob);
    if (blob)
        aug_release(blob);

    if (result < 0) {

        /* Examples show that PyExc_RuntimeError does not need to be
           Py_INCREF()-ed. */

        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
getenv_(PyObject* self, PyObject* args)
{
    const char* name, * value;
    PyObject* def = Py_None;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "s|O:getenv", &name, &def))
        return NULL;

    if (!(value = mod_getenv(name, NULL)))
        return incret_(def);

    return Py_BuildValue("s", value);
}

static PyObject*
shutdown_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned flags;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "O!I:shutdown", type_, &sock, &flags))
        return NULL;

    if (mod_shutdown(augpy_getid(sock), flags) < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
tcpconnect_(PyObject* self, PyObject* args)
{
    const char* host, * serv, * sslctx = NULL;
    PyObject* ob = NULL, * sock;
    augpy_box* box;
    mod_rint cid;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "ss|zO:tcpconnect", &host, &serv, &sslctx,
                          &ob))
        return NULL;

    if (!(box = augpy_boxhandle(type_, 0, ob)))
        return NULL;

    /* mod_tcpconnect() takes ownership. */

    cid = mod_tcpconnect(host, serv, sslctx, (aug_object*)box);
    aug_release(box);

    if (cid < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    sock = box->vtbl_->unbox_(box);
    augpy_setid(sock, (mod_id)cid);
    return sock;
}

static PyObject*
tcplisten_(PyObject* self, PyObject* args)
{
    const char* host, * serv, * sslctx = NULL;
    PyObject* ob = NULL, * sock;
    augpy_box* box;
    mod_rint lid;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "ss|zO:tcplisten", &host, &serv, &sslctx,
                          &ob))
        return NULL;

    if (!(box = augpy_boxhandle(type_, 0, ob)))
        return NULL;

    /* mod_tcplisten() takes ownership. */

    lid = mod_tcplisten(host, serv, sslctx, (aug_object*)box);
    aug_release(box);

    if (lid < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    sock = box->vtbl_->unbox_(box);
    augpy_setid(sock, (mod_id)lid);
    return sock;
}

static PyObject*
send_(PyObject* self, PyObject* args)
{
    PyObject* sock, * buf;
    aug_blob* blob;
    mod_result result;

    /* The object's reference count is not increased. */

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

    result = mod_sendv(augpy_getid(sock), blob);
    aug_release(blob);

    if (result < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
setrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned ms, flags;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "O!II:setrwtimer",
                          type_, &sock, &ms, &flags))
        return NULL;

    if (mod_setrwtimer(augpy_getid(sock), ms, flags) < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    return incret_(Py_None);
}

static PyObject*
resetrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned ms, flags;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "O!II:resetrwtimer",
                          type_, &sock, &ms, &flags))
        return NULL;

    switch (mod_resetrwtimer(augpy_getid(sock), ms, flags)) {
    case MOD_FAILERROR:
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    case MOD_FAILNONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
cancelrwtimer_(PyObject* self, PyObject* args)
{
    PyObject* sock;
    unsigned flags;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "O!I:cancelrwtimer",
                          type_, &sock, &flags))
        return NULL;

    switch (mod_cancelrwtimer(augpy_getid(sock), flags)) {
    case MOD_FAILERROR:
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    case MOD_FAILNONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
settimer_(PyObject* self, PyObject* args)
{
    unsigned ms;
    PyObject* ob = NULL, * timer;
    augpy_box* box;
    mod_rint tid;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "I|O:settimer", &ms, &ob))
        return NULL;

    if (!(box = augpy_boxhandle(type_, 0, ob)))
        return NULL;

    /* mod_settimer() takes ownership. */

    tid = mod_settimer(ms, (aug_object*)box);
    aug_release(box);

    if (tid < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    }

    timer = box->vtbl_->unbox_(box);
    augpy_setid(timer, (mod_id)tid);
    return timer;
}

static PyObject*
resettimer_(PyObject* self, PyObject* args)
{
    PyObject* timer;
    unsigned ms;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "O!I:resettimer", type_, &timer, &ms))
        return NULL;

    switch (mod_resettimer(augpy_getid(timer), ms)) {
    case MOD_FAILERROR:
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    case MOD_FAILNONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
canceltimer_(PyObject* self, PyObject* args)
{
    PyObject* timer;

    /* The object's reference count is not increased. */

    if (!PyArg_ParseTuple(args, "O!:canceltimer", type_, &timer))
        return NULL;

    switch (mod_canceltimer(augpy_getid(timer))) {
    case MOD_FAILERROR:
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
        return NULL;
    case MOD_FAILNONE:
        return incret_(Py_False);
    }

    return incret_(Py_True);
}

static PyObject*
emit_(PyObject* self, PyObject* args)
{
    const char* type;
    const char* buf;
    int len;
    mod_result result;

    if (!PyArg_ParseTuple(args, "s|z#:emit", type, &buf, &len))
        return NULL;

    if (mod_emit(type, buf, len) < 0) {
        PyErr_SetString(PyExc_RuntimeError, mod_geterror());
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
        "emit", emit_, METH_VARARGS,
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
