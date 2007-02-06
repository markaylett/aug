from augas import *
import log

# settimer()

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    settimer(sname, 1000, "our timer")

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
    if timer.user != "our timer":
        log.error("unexpected user in expire()")
    stop()
