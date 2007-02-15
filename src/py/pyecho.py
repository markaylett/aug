from augas import *
import log

def start(sname):
    tcplisten(sname, "0.0.0.0", getenv("session.pyecho.serv"), None)

def data(sock, buf):
    log.info("echoing data")
    send(sock, buf)
