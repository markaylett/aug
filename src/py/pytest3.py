from augpy import *
import log

# getenv()

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    if getenv("pytest3") != "test value":
        error("unexpected value from getenv()")
    if getenv("badname") != None:
        error("unexpected value from getenv()")
    if getenv("badname", 101) != 101:
        error("unexpected value from getenv()")
    stopall()
