from augas import *
import log

# getenv()

def destroy(sname):
    log.debug("destroy(): %s" % sname)

def create(sname):
    log.debug("create(): %s" % sname)
    if getenv("pytest3") != "test value":
        error("unexpected value from getenv()")
    stop()
