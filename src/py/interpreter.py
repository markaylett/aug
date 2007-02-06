import re

Quit = object()

class Interpreter:
    def __init__(self, cmds):
        self.cmds = cmds

    def __call__(self, line):
        line = line.strip()
        if len(line) == 0:
            return None
        toks = re.split(r"\s+", line.strip())
        name = toks[0].lower()
        if name == "exit" or name == "quit":
            return Quit
        args = toks[1:]
        try:
            s = "OK: " + apply(self.cmds[name], args)
        except KeyError:
            s = "ER: invalid command: %s" % line
        except TypeError:
            s = "ER: invalid arguments: %s" % line
        return s
