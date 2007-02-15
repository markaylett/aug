from augas import *
import log

# settimer()

def stop(sname):
    log.debug("stop(): %s" % sname)

def start(sname):
    log.debug("start(): %s" % sname)
    settimer(sname, 1000, "our timer")

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
    if timer.user != "our timer":
        log.error("unexpected user in expire()")
    stopall()
