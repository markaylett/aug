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
	augsys-basetest \
	augsys-errinfotest \
	augsys-logtest \
	augsys-mcasttest \
	augsys-muxertest \
	augsys-sockettest \
	augsys-stringtest \
	augsys-timetest

TESTS = \
	augsys-basetest \
	augsys-errinfotest \
	augsys-logtest \
	augsys-mcasttest \
	augsys-muxertest \
	augsys-sockettest \
	augsys-stringtest \
	augsys-timetest

augsys_basetest_SOURCES = basetest.c
augsys_basetest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_errinfotest_SOURCES = errinfotest.c
augsys_errinfotest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_logtest_SOURCES = logtest.c
augsys_logtest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_mcasttest_SOURCES = mcasttest.c
augsys_mcasttest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_muxertest_SOURCES = muxertest.c
augsys_muxertest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_sockettest_SOURCES = sockettest.c
augsys_sockettest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_stringtest_SOURCES = stringtest.c
augsys_stringtest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

augsys_timetest_SOURCES = timetest.c
augsys_timetest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la
