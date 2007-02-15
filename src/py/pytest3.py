from augas import *
import log

# getenv()

def stop(sname):
    log.debug("stop(): %s" % sname)

def start(sname):
    log.debug("start(): %s" % sname)
    if getenv("pytest3") != "test value":
        error("unexpected value from getenv()")
    stopall()
