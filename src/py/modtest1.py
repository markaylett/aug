from augas import *
import log

# Stop after one second.

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    settimer(sname, 0, 1000, None)

def expire(sname, id, user, ms):
    stop()
    return None
