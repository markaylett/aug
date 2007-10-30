include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	osx \
	posix \
	win32

modincludedir = $(includedir)/augsys
modinclude_HEADERS = \
	barrier.h \
	base.h \
	config.h \
	debug.h \
	defs.h \
	dlfcn.h \
	endian.h \
	errinfo.h \
	errno.h \
	limits.h \
	lock.h \
	log.h \
	mcast.h \
	mmap.h \
	mutex.h \
	muxer.h \
	queue.h \
	socket.h \
	string.h \
	time.h \
	types.h \
	uio.h \
	unistd.h \
	utility.h \
	windows.h

noinst_HEADERS = \
	tls_.h

lib_LTLIBRARIES = libaugsys.la

libaugsys_la_SOURCES = \
	barrier.c \
	base.c \
	config.c \
	debug.c \
	defs.c \
	dlfcn.c \
	endian.c \
	errinfo.c \
	errno.c \
	limits.c \
	lock.c \
	log.c \
	mcast.c \
	mmap.c \
	mutex.c \
	muxer.c \
	socket.c \
	string.c \
	time.c \
	types.c \
	uio.c \
	unistd.c \
	utility.c \
	windows.c \
	tls_.c

libaugsys_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugsys_la_LIBADD =

check_PROGRAMS = \
	basetest \
	errinfotest \
	logtest \
	mcasttest \
	muxertest \
	sockettest \
	stringtest \
	timetest

TESTS = \
	basetest \
	errinfotest \
	logtest \
	mcasttest \
	muxertest \
	sockettest \
	stringtest \
	timetest

basetest_SOURCES = basetest.c
basetest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

errinfotest_SOURCES = errinfotest.c
errinfotest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

logtest_SOURCES = logtest.c
logtest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

mcasttest_SOURCES = mcasttest.c
mcasttest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

muxertest_SOURCES = muxertest.c
muxertest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

sockettest_SOURCES = sockettest.c
sockettest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

stringtest_SOURCES = stringtest.c
stringtest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

timetest_SOURCES = timetest.c
timetest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la
