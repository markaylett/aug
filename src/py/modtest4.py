from augas import *
from buffer import *
import log

# tcplisten(), tcpconnect()

(listener, server, client) = (None, None, None)

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    global client, listener
    log.debug("init(): %s" % sname)
    listener = tcplisten(sname, "0.0.0.0", "1234", None)
    client = tcpconnect(sname, "127.0.0.1", "1234", None)

def closed(sname, id, user):
    log.debug("closed(): %s" % sname)

def accept(sname, cid, user, addr, port):
    global server
    log.debug("accept(): %s" % sname)
    server = cid
    return Buffer()

def connected(sname, cid, user, addr, port):
    global server
    log.debug("connected(): %s" % sname)
    send(sname, server, "hello, world!\n", SNDPEER)
    return Buffer()

def data(sname, cid, user, buf):
    log.debug("data(): %s" % sname)
    for line in user.lines(str(buf)):
        if line != "hello, world!":
            log.error("unexpected line from data()")
        if cid == server:
            send(sname, cid, line + "\n", SNDPEER)
        else:
            stop()
