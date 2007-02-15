from augas import *
from buffer import *
import log

# tcplisten(), tcpconnect()

(client, listener, server) = (None, None, None)

def destroy(sname):
    log.debug("destroy(): %s" % sname)

def create(sname):
    global client, listener
    log.debug("create(): %s" % sname)
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
