from augas import *
import log

# settimer()

def destroy(sname):
    log.debug("destroy(): %s" % sname)

def create(sname):
    log.debug("create(): %s" % sname)
    settimer(sname, 1000, "our timer")

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
    if timer.user != "our timer":
        log.error("unexpected user in expire()")
    stop()
