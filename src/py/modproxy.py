from augas import *
import log

pairs = {}

def init(sname):
    log.info("binding proxy listener")
    tcplisten(sname, "0.0.0.0", getenv("session.modproxy.serv"), None)

def closed(sock):
    global pairs
    if pairs.has_key(sock.id):
        to = pairs[sock.id]
        log.info("closing proxy pair: (%d, %d)" % (sock.id, to))
        del pairs[sock.id]
        del pairs[to]
        shutdown(to)

def accept(sock, addr, port):
    global pairs
    log.info("opening proxy pair")
    # connected() will always be called after tcpconnect() returns.
    to = tcpconnect(sock.sname, "localhost", getenv("session.modproxy.to"), sock.id)
    pairs[sock.id] = to
    pairs[to] = sock.id

def connected(sock, addr, port):
    log.info("proxy pair established")

def data(sock, buf):
    # It is safe to send to a connection that has not been fully established.
    if pairs.has_key(sock.id):
        send(pairs[sock.id], buf)
