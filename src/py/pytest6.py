from augas import *
import log

# Return false from accept.

def stop():
    log.debug("stop()")

def start(sname):
    global client, listener
    log.debug("start(): %s" % sname)
    tcplisten("0.0.0.0", "1234", None)
    tcpconnect("127.0.0.1", "1234", None)

def closed(sock):
    log.debug("closed(): %s" % sock)

def accept(sock, addr, port):
    log.debug("accept(): %s" % sock)
    return False

def connected(sock, addr, port):
    log.debug("connected(): %s" % sock)
