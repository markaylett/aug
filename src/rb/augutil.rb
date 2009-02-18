# Copyright (c) 2004, 2005, 2006, 2007, 2008, 2009 Mark Aylett <mark.aylett@gmail.com>
#
# This file is part of Aug written by Mark Aylett.
#
# Aug is released under the GPL with the additional exemption that compiling,
# linking, and/or using OpenSSL is allowed.
#
# Aug is free software; you can redistribute it and/or modify it under the
# terms of the GNU General Public License as published by the Free Software
# Foundation; either version 2 of the License, or (at your option) any later
# version.
#
# Aug is distributed in the hope that it will be useful, but WITHOUT ANY
# WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
# FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
# details.
#
# You should have received a copy of the GNU General Public License along with
# this program; if not, write to the Free Software Foundation, Inc., 51
# Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

module AugUtil

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
end
