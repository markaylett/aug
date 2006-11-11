#include <Python.h>
#include <augas.h>

#include <stdlib.h>

static PyObject*
getmodule_(const char* name)
{
    PyObject* s, * m;
    s = PyString_FromString(name);
    m = PyImport_Import(s);
    Py_DECREF(s);
    return m;
}

static PyObject*
getmethod_(PyObject* module, const char* name)
{
    PyObject* fn = PyObject_GetAttrString(module, name);
    if (fn && !PyCallable_Check(fn)) {
        Py_DECREF(fn);
        fn = NULL;
    }
    return fn;
}

static char modname_[256] = "";
static const struct augas_host* host_ = NULL;

/**
   Python objects.
*/

static PyObject* pyaugas_ = NULL;
static PyObject* pymodule_ = NULL;

static PyObject* pyload_ = NULL;
static PyObject* pyunload_ = NULL;
static PyObject* pyclose_ = NULL;
static PyObject* pyopen_ = NULL;
static PyObject* pydata_ = NULL;
static PyObject* pyrdexpire_ = NULL;
static PyObject* pywrexpire_ = NULL;
static PyObject* pystop_ = NULL;
static PyObject* pyevent_ = NULL;
static PyObject* pyexpire_ = NULL;
static PyObject* pyreconf_ = NULL;

static PyObject*
pyerror_(PyObject *self, PyObject *args)
{
    if (!PyArg_ParseTuple(args, ":error"))
        return NULL;
    return Py_BuildValue("s", host_->error_(modname_));
}

static PyObject*
pygetenv_(PyObject *self, PyObject *args)
{
    const char* s;
    if (!PyArg_ParseTuple(args, "s:getenv", &s))
        return NULL;
    return Py_BuildValue("s", host_->getenv_(modname_, s));
}

static PyObject*
pywritelog_(PyObject *self, PyObject *args)
{
    int i;
    const char* s;
    if (!PyArg_ParseTuple(args, "is:writelog", &i, &s))
        return NULL;
    host_->writelog_(modname_, i, "%s", s);
    Py_RETURN_NONE;
}

static PyObject*
pypost_(PyObject *self, PyObject *args)
{
    int type;
    PyObject* arg;
    if (!PyArg_ParseTuple(args, "iO:post", &type, &arg))
        return NULL;
    Py_INCREF(arg);
    host_->post_(modname_, type, arg);
    Py_RETURN_NONE;
}

static PyObject*
pysettimer_(PyObject *self, PyObject *args)
{
    int id;
    unsigned ms;
    PyObject* arg;
    if (!PyArg_ParseTuple(args, "iIO:settimer", &id, &ms, &arg))
        return NULL;
    Py_INCREF(arg);
    host_->settimer_(modname_, id, ms, arg);
    Py_RETURN_NONE;
}

static PyObject*
pyresettimer_(PyObject *self, PyObject *args)
{
    int id;
    unsigned ms;
    if (!PyArg_ParseTuple(args, "iI:resettimer", &id, &ms))
        return NULL;
    host_->resettimer_(modname_, id, ms);
    Py_RETURN_NONE;
}

static PyObject*
pycanceltimer_(PyObject *self, PyObject *args)
{
    int id;
    if (!PyArg_ParseTuple(args, "i:resettimer", &id))
        return NULL;
    host_->canceltimer_(modname_, id);
    Py_RETURN_NONE;
}

static PyObject*
pyshutdown_(PyObject *self, PyObject *args)
{
    unsigned sid;
    if (!PyArg_ParseTuple(args, "I:shutdown", &sid))
        return NULL;
    host_->shutdown_(sid);
    Py_RETURN_NONE;
}

static PyObject*
pysend_(PyObject *self, PyObject *args)
{
    unsigned sid;
    const char* buf;
    int size;
    unsigned flags;
    if (!PyArg_ParseTuple(args, "Is#I:send", &sid, &buf, &size, &flags))
        return NULL;
    host_->send_(sid, buf, size, flags);
    Py_RETURN_NONE;
}

static PyObject*
pysetrwtimer_(PyObject *self, PyObject *args)
{
    unsigned sid, ms, flags;
    if (!PyArg_ParseTuple(args, "III:setrwtimer", &sid, &ms, &flags))
        return NULL;
    host_->setrwtimer_(sid, ms, flags);
    Py_RETURN_NONE;
}

static PyObject*
pyresetrwtimer_(PyObject *self, PyObject *args)
{
    unsigned sid, ms, flags;
    if (!PyArg_ParseTuple(args, "III:resetrwtimer", &sid, &ms, &flags))
        return NULL;
    host_->resetrwtimer_(sid, ms, flags);
    Py_RETURN_NONE;
}

static PyObject*
pycancelrwtimer_(PyObject *self, PyObject *args)
{
    unsigned sid, flags;
    if (!PyArg_ParseTuple(args, "II:cancelrwtimer", &sid, &flags))
        return NULL;
    host_->cancelrwtimer_(sid, flags);
    Py_RETURN_NONE;
}

static PyMethodDef pymethods_[] = {
    {
        "error", pyerror_, METH_VARARGS,
        "TODO"
    },
    {
        "getenv", pygetenv_, METH_VARARGS,
        "TODO"
    },
    {
        "writelog", pywritelog_, METH_VARARGS,
        "TODO"
    },
    {
        "post", pypost_, METH_VARARGS,
        "TODO"
    },
    {
        "settimer", pysettimer_, METH_VARARGS,
        "TODO"
    },
    {
        "resettimer", pyresettimer_, METH_VARARGS,
        "TODO"
    },
    {
        "canceltimer", pycanceltimer_, METH_VARARGS,
        "TODO"
    },
    {
        "shutdown", pyshutdown_, METH_VARARGS,
        "TODO"
    },
    {
        "send", pysend_, METH_VARARGS,
        "TODO"
    },
    {
        "setrwtimer", pysetrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "resetrwtimer", pyresetrwtimer_, METH_VARARGS,
        "TODO"
    },
    {
        "cancelrwtimer", pycancelrwtimer_, METH_VARARGS,
        "TODO"
    },
    { NULL, NULL, 0, NULL }
};

#define DECREF_(x)                              \
    if (x) {                                    \
        Py_DECREF(x);                           \
        x = NULL;                               \
    }

static void
pyfree_(void)
{
    DECREF_(pyreconf_);
    DECREF_(pyexpire_);
    DECREF_(pyevent_);
    DECREF_(pystop_);
    DECREF_(pywrexpire_);
    DECREF_(pyrdexpire_);
    DECREF_(pydata_);
    DECREF_(pyopen_);
    DECREF_(pyclose_);
    DECREF_(pyunload_);
    DECREF_(pyload_);
    DECREF_(pymodule_);
    DECREF_(pyaugas_);

    if (Py_IsInitialized())
        Py_Finalize();
}

static int
pycreate_(void)
{
    Py_Initialize();
    pyaugas_ = Py_InitModule("augas", pymethods_);
    pymodule_ = getmodule_(modname_);

    PyModule_AddIntConstant(pyaugas_, "LOGCRIT", AUGAS_LOGCRIT);
    PyModule_AddIntConstant(pyaugas_, "LOGERROR", AUGAS_LOGERROR);
    PyModule_AddIntConstant(pyaugas_, "LOGWARN", AUGAS_LOGWARN);
    PyModule_AddIntConstant(pyaugas_, "LOGNOTICE", AUGAS_LOGNOTICE);
    PyModule_AddIntConstant(pyaugas_, "LOGINFO", AUGAS_LOGINFO);
    PyModule_AddIntConstant(pyaugas_, "LOGDEBUG", AUGAS_LOGDEBUG);

    PyModule_AddIntConstant(pyaugas_, "TIMRD", AUGAS_TIMRD);
    PyModule_AddIntConstant(pyaugas_, "TIMWR", AUGAS_TIMWR);
    PyModule_AddIntConstant(pyaugas_, "TIMBOTH", AUGAS_TIMBOTH);

    PyModule_AddIntConstant(pyaugas_, "SESSELF", AUGAS_SESSELF);
    PyModule_AddIntConstant(pyaugas_, "SESOTHER", AUGAS_SESOTHER);
    PyModule_AddIntConstant(pyaugas_, "SESALL", AUGAS_SESALL);

    pyload_ = getmethod_(pymodule_, "load");
    pyunload_ = getmethod_(pymodule_, "unload");
    pyclose_ = getmethod_(pymodule_, "close");
    pyopen_ = getmethod_(pymodule_, "open");
    pydata_ = getmethod_(pymodule_, "data");
    pyrdexpire_ = getmethod_(pymodule_, "rdexpire");
    pywrexpire_ = getmethod_(pymodule_, "wrexpire");
    pystop_ = getmethod_(pymodule_, "stop");
    pyevent_ = getmethod_(pymodule_, "event");
    pyexpire_ = getmethod_(pymodule_, "expire");
    pyreconf_ = getmethod_(pymodule_, "reconf");

    return 0;
}

static void
close_(const struct augas_session* s)
{
    PyObject_CallFunction(pyclose_, "IO", s->sid_, s->user_);
    Py_DECREF((PyObject*)s->user_);
}

static int
open_(struct augas_session* s, const char* serv, const char* peer)
{
    PyObject* x = PyObject_CallFunction(pyopen_, "Iss", s->sid_, serv, peer);
    s->user_ = x;
    return 0;
}

static int
data_(const struct augas_session* s, const char* buf, size_t size)
{
    PyObject* x = PyBuffer_FromMemory((void*)buf, size);
    PyObject_CallFunction(pydata_, "IOO", s->sid_, s->user_, x);
    Py_DECREF(x);
    return 0;
}

static int
rdexpire_(const struct augas_session* s, unsigned* ms)
{
    PyObject* x = PyInt_FromLong(*ms);
    PyObject_CallFunction(pyrdexpire_, "IOO", s->sid_, s->user_, x);
    *ms = PyInt_AsLong(x);
    Py_DECREF(x);
    return 0;
}

static int
wrexpire_(const struct augas_session* s, unsigned* ms)
{
    PyObject* x = PyInt_FromLong(*ms);
    PyObject_CallFunction(pywrexpire_, "IOO", s->sid_, s->user_, x);
    *ms = PyInt_AsLong(x);
    Py_DECREF(x);
    return 0;
}

static int
stop_(const struct augas_session* s)
{
    PyObject_CallFunction(pystop_, "IO", s->sid_, s->user_);
    return 0;
}

static int
event_(int type, void* arg)
{
    PyObject* x = arg;
    PyObject_CallFunction(pyevent_, "iO", type, x);
    Py_DECREF(x);
    return 0;
}

static int
expire_(void* arg, unsigned id, unsigned* ms)
{
    PyObject* x = arg;
    PyObject* y = PyInt_FromLong(*ms);
    PyObject_CallFunction(pyexpire_, "OIO", x, id, y);
    *ms = PyInt_AsLong(y);
    Py_DECREF(y);
    if (0 == *ms) {
        Py_DECREF(x);
    }
    return 0;
}

static int
reconf_(void)
{
    PyObject_CallFunction(pyreconf_, NULL);
    return 0;
}

static const struct augas_module fntable_ = {
    close_,
    open_,
    data_,
    rdexpire_,
    wrexpire_,
    stop_,
    event_,
    expire_,
    reconf_
};

static const struct augas_module*
load_(const char* modname, const struct augas_host* host)
{
    /**
       Fail if module has already been loaded.
    */

    if ('\0' != *modname_)
        return NULL;

    strcpy(modname_, modname);
    host_ = host;

    if (pycreate_() < 0)
        return NULL;

    PyObject_CallFunction(pyload_, NULL);
    return &fntable_;
}

static void
unload_(void)
{
    PyObject_CallFunction(pyunload_, NULL);
    pyfree_();
    host_ = 0;
}

AUGAS_MODULE(load_, unload_)
