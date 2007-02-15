from augas import *
import log

# stop(), Object()

def stop(sname):
    log.debug("stop(): %s" % sname)

def start(sname):
    log.debug("start(): %s" % sname)
    o = Object("test", 101, "our object")
    log.debug("str(): %s" % o)
    if o.sname != "test":
        log.error("object returned unexpected sname")
    if o.id != 101:
        log.error("object returned unexpected id")
    if o.user != "our object":
        log.error("object returned unexpected user")
    o.user = "new user"
    if o.user != "new user":
        log.error("object returned unexpected user")
    if o != Object("other", 101, None):
        log.error("comparison operator failed")
    stopall()
