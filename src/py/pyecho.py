from augas import *
import log

def start(sname):
    tcplisten("0.0.0.0", getenv("service.pyecho.serv"), None)

def data(sock, buf):
    send(sock, buf)
