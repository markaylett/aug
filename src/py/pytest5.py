from augas import *
from buffer import *
import log

# tcplisten(), tcpconnect()

(client, listener, server) = (None, None, None)

def term(sname):
    log.debug("term(): %s" % sname)

def init(sname):
    global client, listener
    log.debug("init(): %s" % sname)
    listener = tcplisten(sname, "0.0.0.0", "1234", None)
    client = tcpconnect(sname, "127.0.0.1", "1234", None)

def closed(sock):
    log.debug("closed(): %s" % sock)

def accept(sock, addr, port):
    global server
    log.debug("accept(): %s" % sock)
    sock.user = Buffer()
    server = sock

def connected(sock, addr, port):
    global server
    log.debug("connected(): %s" % sock)
    sock.user = Buffer()
    send(server, "hello, world!\n")

def data(sock, buf):
    log.debug("data(): %s" % sock)
    for line in sock.user.lines(str(buf)):
        if line != "hello, world!":
            log.error("unexpected line from data()")
        if sock == server:
            send(sock, line + "\n")
        else:
            stop()
