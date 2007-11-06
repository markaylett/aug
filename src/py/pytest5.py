from augpy import *
from augutil import *
import log

# tcplisten(), tcpconnect()

(client, listener, server) = (None, None, None)

def stop():
    log.debug("stop()")

def start(sname):
    global client, listener
    log.debug("start(): %s" % sname)
    listener = tcplisten("0.0.0.0", "1234", None)
    client = tcpconnect("127.0.0.1", "1234", None)

def closed(sock):
    log.debug("closed(): %s" % sock)

def accepted(sock, addr, port):
    global server
    log.debug("accepted(): %s" % sock)
    sock.user = LineParser()
    server = sock

def connected(sock, addr, port):
    global server
    log.debug("connected(): %s" % sock)
    sock.user = LineParser()
    send(server, "hello, world!\n")

def data(sock, buf):
    log.debug("data(): %s" % sock)
    for line in sock.user.parse(str(buf)):
        if line != "hello, world!":
            log.error("unexpected line from data()")
        if sock == server:
            send(sock, line + "\n")
        else:
            stopall()
