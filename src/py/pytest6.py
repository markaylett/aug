from augpy import *
import log

# Return false from accepted().

def stop():
    log.debug("stop()")

def start(sname):
    log.debug("start(): %s" % sname)
    tcplisten("0.0.0.0", "1234", None, None)
    tcpconnect("127.0.0.1", "1234", None, None)

def closed(sock):
    log.debug("closed(): %s" % sock)

def accepted(sock, name):
    log.debug("accepted(): %s" % sock)
    return False

def connected(sock, name):
    log.debug("connected(): %s" % sock)
