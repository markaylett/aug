from augas import *
import log

# post(), invoke()

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    invoke(sname, 1, str(101))
    post(sname, 2, str(202))
    post(sname, 3, None)

def event(sname, type, user):
    log.debug("event(): %s" % sname)
    if type == 1:
        if int(user) != 101:
            log.error("unexpected user data")
    elif type == 2:
        if int(user) != 202:
            log.error("unexpected user data")
    elif type == 3:
        if user is not None:
            log.error("unexpected user data")
        stop()
    else:
        log.error("unexpected type")
