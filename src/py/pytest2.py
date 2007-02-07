from augas import *
import log

# post(), delegate()

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    delegate(sname, 1, 101)
    post(sname, 2, 202)

def event(sname, type, user):
    log.debug("event(): %s" % sname)
    if type == 1:
        if user != 101:
            log.error("unexpected user data")
    elif type == 2:
        if user != 202:
            log.error("unexpected user data")
        stop()
    else:
        log.error("unexpected type")
