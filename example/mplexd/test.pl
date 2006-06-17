#!/usr/bin/perl -w

use strict;
use IO::Socket;
use POSIX qw(:sys_wait_h);

my $HOST = 'localhost';
my $PORT = 8080;
my $done = 0;
my $num = 0;
my $tot = 0;

sub reaper {

    my $pid;
    for (;;) {
        print "Reaping children\n";
        $pid = waitpid(-1, WNOHANG);
        last if !$pid;
        --$num;
        $tot += 10000;
        print "Total: $tot\n";
    }
    $SIG{CHLD} = \&reaper;
}

sub quit {
    $done = 1;
}

$SIG{CHLD} = \&reaper;
$SIG{INT} = \&quit;

while (!$done) {

    if ($num == 10) {
        print "Sleeping\n";
        sleep 60;
        next;
    }

    my $pid = fork;
    die "Failed to fork child: $!\n"
        unless defined $pid;

    if ($pid) {
        ++$num;
        next;
    }

    my $socket = IO::Socket::INET->new(PeerAddr => $HOST,
                                       PeerPort => $PORT,
                                       Proto => "tcp",
                                       Type => SOCK_STREAM)
        or die "Failed to connect: $!\n";

    print "Connection established\n";
    my $i;
    for ($i = 0; $i < 10000; ++$i) {
        print $socket "<ping/>\n";
        my $answer = <$socket>;
    }

    close($socket);
    print "Connection dropped\n";
    exit 0;
}
