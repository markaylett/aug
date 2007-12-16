from augpy import *
import log

# stop(), Handle()

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    o = Handle(101, "our object")
    log.debug("str(): %s" % o)
    if o.id != 101:
        log.error("object returned unexpected id")
    if o.user != "our object":
        log.error("object returned unexpected user")
    o.user = "new user"
    if o.user != "new user":
        log.error("object returned unexpected user")
    if o != Handle(101, None):
        log.error("comparison operator failed")
    stopall()
