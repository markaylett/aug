#!/usr/bin/perl -w
use strict;

my $CONFIG = 'config.sh';

my ($LINUX, $LINUX_MINGW, $MINGW, $CYGWIN, $CYGWIN_MINGW, $OTHER)
    = (1 .. 6);

my %TOOLSET = (
               $LINUX => 'linux build',
               $LINUX_MINGW => 'linux build for mingw host',
               $MINGW => 'mingw build',
               $CYGWIN => 'cygwin build',
               $CYGWIN_MINGW => 'cygwin build for mingw host',
               $OTHER => 'other (posix)'
               );

my ($SHARED_ONLY, $STATIC_ONLY, $BOTH) = (1 .. 3);

my %LIBTYPE = (
               $BOTH => 'both',
               $SHARED_ONLY => 'shared only',
               $STATIC_ONLY => 'static only'
               );

sub trim {
    my $s = shift;
    $s =~ s/^\s+//;
    $s =~ s/\s+$//;
    return $s;
}

sub is {
    my $s = lc (shift);
    return ($s eq '1'
            || $s eq 'y'
            || $s eq 'yes'
            || $s eq 'true') ? 1 : 0;
}

sub valueask {
    my ($prompt, $default) = @_;
    print "$prompt [$default]: ";
    my $answer = <STDIN>;
    unless ($answer) {
        print "(quit)\n";
        exit 0;
    }
    chomp $answer;
    $answer = trim $answer;
    $answer = $default unless length $answer;
    return $answer;
}

sub listask {
    my ($prompt, $default, $pairs) = @_;
    print "\n";
    foreach my $name (sort (keys %$pairs)) {
        if ($name eq $default) {
            print "[$name]\t$pairs->{$name}\n";
        } else {
            print " $name\t$pairs->{$name}\n";
        }
    }
    print "\n";
    my $answer;
    do {
        $answer = valueask ($prompt, $default);
        # Loop while bad answer.
    } while (!exists $pairs->{$answer});
    return $answer;
}

my (
    $win32,
    $prefix,
    $toolset,
    $maintainer,
    $gcc,
    $strict,
    $debug,
    $multicast,
    $python,
    $ruby,
    $smp,
    $ssl,
    $threads,
    $libtype
    );

$win32 = (exists $ENV{OS} && $ENV{OS} =~ /windows/i);

if (defined $ENV{AUG_HOME}) {
    $prefix = $ENV{AUG_HOME};
    $prefix =~ s!\\!/!g;
} else {
    $prefix = $win32 ? 'c:/aug' : $ENV{HOME};
}

if ($win32) {

    if ($^O =~ /cygwin/i) {
        $toolset = $CYGWIN_MINGW;
    } else {
        $toolset = $MINGW;
    }

} else {
    $toolset = $LINUX;
}

# Turn off line-buffering.

$| = 1;

# Collect the input.

$prefix = valueask ("prefix directory", $prefix);
$prefix =~ s{^~([^/]*)}
{
    $1 ? (getpwnam($1))[7]
        : ($ENV{HOME} || $ENV{LOGDIR} || (getpwuid($>))[7])
    }ex;

$toolset = listask ("compiler toolset", $toolset, \%TOOLSET);
$maintainer = valueask ("maintainer mode", 'n');
if ($CYGWIN_MINGW == $toolset) {
    $gcc = 'y';
} else {
    $gcc = valueask ("GCC compiler", 'y');
}
$strict = valueask ("strict build", 'n');
$debug = valueask ("debug build", 'n');
$multicast = valueask ("multicast support", 'y');
$python = valueask ("python support", 'n');
$ruby = valueask ("ruby support", 'n');
$smp = valueask ("smp support", 'y');
$ssl = valueask ("ssl support", 'y');
$threads = valueask ("thread support", 'y');
$libtype = listask ("library type", $BOTH, \%LIBTYPE);

my (
    $flags,
    $cflags,
    $cxxflags,
    $options
    );

if (is $gcc) {
    my $gprof = valueask ("gprof build", 'n');
    $flags = (is $gprof) ? '-pg' : '';
    $flags .= (is $debug) ? ' -ggdb' : ' -O3 -DNDEBUG';
    $flags .= ' -Wno-long-long -fno-strict-aliasing -Wno-unused-value';
    $cflags = "$flags";
    $cxxflags = "$flags -Wno-deprecated -Wno-unused-variable";
    if (is $strict) {
        $cflags = "-Wall -pedantic $cflags";
        $cxxflags = "-Wall -pedantic $cxxflags";
    }
} else {
    $flags = (is $debug) ? '-g' : '-O -DNDEBUG';
    $cflags = $flags;
    $cxxflags = $flags;
}

# Write script.

open (FILE, ">$CONFIG")
    or die "open() failed: $!\n";

print FILE "#!/bin/sh\n\n";
print FILE "AUG_HOME='$prefix'; export AUG_HOME\n";

$options = "--prefix=\$AUG_HOME";
$options .= " \\\n\t--enable-maintainer-mode"
    if is $maintainer;
$options .= " \\\n\t--disable-multicast"
    unless is $multicast;
$options .= " \\\n\t--enable-python"
    if is $python;
$options .= " \\\n\t--enable-ruby"
    if is $ruby;
$options .= " \\\n\t--disable-smp"
    unless is $smp;
$options .= " \\\n\t--disable-ssl"
    unless is $ssl;
$options .= " \\\n\t--disable-threads"
    unless is $threads;

if ($SHARED_ONLY == $libtype) {
    $options .= " \\\n\t--disable-static";
} elsif ($STATIC_ONLY == $libtype) {
    $options .= " \\\n\t--disable-shared";
}

if ($CYGWIN_MINGW == $toolset) {
    print FILE "CC='gcc -mno-cygwin'; export CC\n";
    print FILE "CXX='g++ -mno-cygwin'; export CXX\n";
} elsif (2 == $toolset) {
    $options .= " \\\n\t--build=i586-pc-linux-gnu";
    $options .= " \\\n\t--host=i586-mingw32msvc";
}

print FILE "CFLAGS='$cflags'; export CFLAGS\n";
print FILE "CXXFLAGS='$cxxflags'; export CXXFLAGS\n";
print FILE "rm -f config.cache && sh ./configure \\\n\t$options\n";

close FILE;
print "\nrun script as follows:\n\$ sh $CONFIG\n";
