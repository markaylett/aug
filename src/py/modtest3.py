from augas import *
import log

# getenv()

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    log.debug("init(): %s" % sname)
    if getenv("modtest3") != "test value":
        error("unexpected value from getenv()")
    stop()
