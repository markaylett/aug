from augas import *
from cgi import *
from urllib import *
import log


# post(), dispatch()

def destroy(sname):
    log.debug("destroy(): %s" % sname)

def create(sname):
    log.debug("create(): %s" % sname)
    dispatch(sname, "group1", "foo", str(101))
    post(sname, "group2", "application/x-www-form-urlencoded", urlencode({"a": 1}))
    post(sname, sname, "none", None)

def event(sname, frm, type, user):
    log.debug("event(): %s %s" % (sname, user))
    if type == "foo":
        if int(user) != 101:
            log.error("unexpected user data")
        dispatch(sname, frm, "bar", buffer("202"))
    elif type == "bar":
        if int(user) != 202:
            log.error("unexpected user data")
    elif type == "application/x-www-form-urlencoded":
        if int(parse_qs(user)["a"][0]) != 1:
            log.error("unexpected user data")
    elif type == "none":
        if user is not None:
            log.error("unexpected user data")
        stop()
    else:
        log.error("unexpected type")
