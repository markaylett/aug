from augas import *
import log

def start(sname):
    settimer(sname, 5000, None)

def expire(timer, ms):
    stopall()
