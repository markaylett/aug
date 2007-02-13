from augas import *
import log

# post(), invoke()

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    invoke(sname, sname, "foo", str(101))
    post(sname, sname, "bar", buffer("202"))
    post(sname, sname, "none", None)

def event(sname, frm, type, user):
    log.debug("event(): %s" % sname)
    if type == "foo":
        if int(user) != 101:
            log.error("unexpected user data")
    elif type == "bar":
        if int(user) != 202:
            log.error("unexpected user data")
    elif type == "none":
        if user is not None:
            log.error("unexpected user data")
        stop()
    else:
        log.error("unexpected type")
