from augpy import *
import log

def start(sname):
    tcplisten("0.0.0.0", getenv("session.pyecho.serv"), None, None)

def recv(sock, buf):
    send(sock, buf)
