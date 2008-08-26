include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	osx \
	posix \
	win32

modincludedir = $(includedir)/augsys
modinclude_HEADERS = \
	barrier.h \
	base.h \
	chan.h \
	config.h \
	debug.h \
	dlfcn.h \
	endian.h \
	limits.h \
	mcast.h \
	mmap.h \
	muxer.h \
	queue.h \
	socket.h \
	stream.h \
	time.h \
	types.h \
	uio.h \
	unistd.h \
	utility.h \
	windows.h

lib_LTLIBRARIES = libaugsys.la

libaugsys_la_SOURCES = \
	barrier.c \
	base.c \
	chan.c \
	config.c \
	debug.c \
	dlfcn.c \
	endian.c \
	limits.c \
	mcast.c \
	mmap.c \
	muxer.c \
	socket.c \
	stream.c \
	time.c \
	types.c \
	uio.c \
	unistd.c \
	utility.c \
	windows.c

libaugsys_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugsys_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

check_PROGRAMS = \
	basetest \
	chantest \
	mcasttest \
	muxertest \
	sockaddrtest \
	sockettest \
	timetest \
	utilitytest

TESTS = \
	basetest \
	mcasttest \
	muxertest \
	objecttest \
	sockaddrtest \
	sockettest \
	timetest \
	utilitytest

basetest_SOURCES = basetest.c
basetest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

chantest_SOURCES = chantest.c
chantest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

mcasttest_SOURCES = mcasttest.c
mcasttest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

muxertest_SOURCES = muxertest.c
muxertest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

sockaddrtest_SOURCES = sockaddrtest.c
sockaddrtest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

sockettest_SOURCES = sockettest.c
sockettest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

timetest_SOURCES = timetest.c
timetest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

utilitytest_SOURCES = utilitytest.c
utilitytest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la
