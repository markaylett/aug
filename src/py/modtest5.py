from augas import *
import log

# settimer()

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    settimer(sname, 0, 1000, None)

def expire(sname, tid, user, ms):
    log.debug("expire(): %s" % sname)
    stop()
