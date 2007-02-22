from augas import *
import log

# stop(), Object()

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    o = Object(101, "our object")
    log.debug("str(): %s" % o)
    if o.id != 101:
        log.error("object returned unexpected id")
    if o.user != "our object":
        log.error("object returned unexpected user")
    o.user = "new user"
    if o.user != "new user":
        log.error("object returned unexpected user")
    if o != Object(101, None):
        log.error("comparison operator failed")
    stopall()
