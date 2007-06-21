module RbTest1
  def RbTest1.stop
    puts "stop()"
  end
  def RbTest1.start(sname)
    Augrt.writelog(Augrt::LOGINFO, "start(): #{sname} #{$:}")
    o = Augrt::Object.new(101, "test")
    puts o.id
    puts o.user
    o.user = "x"
    puts o.user
    Augrt.dispatch(sname, "foo", "hello")
    Augrt.post(sname, "bar", "again")
    $stdout.flush
    s = Augrt.getenv("sessions")
    puts "sessions: #{s}"
    Augrt.settimer(1000, "our timer")
    sock = Augrt.tcpconnect("localhost", "1245")
    puts "opening: #{sock.id} - ruby"
  end
  def RbTest1.closed(sock)
    puts "closing: #{sock.id} - ruby"
  end
  def RbTest1.event(frm, type, user)
    puts "#{frm}: #{type}"    
    $stdout.flush
  end
  def RbTest1.expire(timer, ms)
    puts "timer: #{timer.user}, #{ms}"
    ms += 1000
    #Augrt.stopall
  end
end
