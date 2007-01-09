class Buffer:
    def __init__(self):
        self.tail = ""

    def lines(self, data):
        data = self.tail + data
        ls = data.split("\n")
        self.tail = ls.pop()
        for l in ls:
            yield l
