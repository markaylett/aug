QUIT = Object.new

def foldl(x, ys)
    ys.each { |y| x = yield(x, y) }
    x
end

class LineParser
    def initialize
        @tail = ''
    end
    def parse(data, &block)
        data = @tail + data
        ls = data.split(/\r?\n/, -1)
        @tail = ls.pop
        ls.each &block
    end
end

class Interpreter
    def initialize(handler)
        @handler = handler
    end
    def interpret(line)
        line.strip!
        return if line.empty?
        toks = line.split(/\s+/)
        name = 'do_' + toks.shift
        begin
            x = @handler.send(name.downcase.intern, *toks)
            return x if x == QUIT
            if x == nil
                x = '+OK'
            elsif !x.kind_of?(String) && x.kind_of?(Enumerable)
                x = foldl("+OK\r\n", x) { |x, y| x + "#{y}\r\n" }
                x += '.'
            else
                x = "+OK #{x}"
            end
        rescue NoMethodError => e
            x = "-ERR invalid command: #{e}"
        rescue ArgumentError => e
            x = "-ERR invalid arguments: #{e}"
        rescue Exception => e
            x = "-ERR exception: #{e}"
        end
        x
    end
end
