from augrt import *
import log

# settimer()

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    settimer(1000, "our timer")

def expire(timer, ms):
    log.debug("expire(): %s" % timer)
    if timer.user != "our timer":
        log.error("unexpected user in expire()")
    stopall()
