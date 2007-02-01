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

def closed(sock):
    log.debug("closed(): %s" % sock.sname)

def accept(sock, addr, port):
    global server
    log.debug("accept(): %s" % sock.sname)
    sock.user = Buffer()
    server = sock.id

def connected(sock, addr, port):
    global server
    log.debug("connected(): %s" % sock.sname)
    sock.user = Buffer()
    send(server, "hello, world!\n")

def data(sock, buf):
    log.debug("data(): %s" % sock.sname)
    for line in sock.user.lines(str(buf)):
        if line != "hello, world!":
            log.error("unexpected line from data()")
        if sock.id == server:
            send(sock.id, line + "\n")
        else:
            stop()
