include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

modincludedir = $(includedir)/augsys
modinclude_HEADERS = \
	base.h \
	config.h \
	debug.h \
	defs.h \
	errinfo.h \
	errno.h \
	inet.h \
	limits.h \
	lock.h \
	log.h \
	mmap.h \
	mplexer.h \
	mutex.h \
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
	base.c \
	config.c \
	debug.c \
	defs.c \
	errinfo.c \
	errno.c \
	inet.c \
	limits.c \
	lock.c \
	log.c \
	mmap.c \
	mplexer.c \
	mutex.c \
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
	-version-info 0:0:0

libaugsys_la_LIBADD =

check_PROGRAMS = \
	augsys-basetest \
	augsys-errinfotest \
	augsys-sockettest

TESTS = \
	augsys-basetest \
	augsys-errinfotest \
	augsys-sockettest

augsys_basetest_SOURCES = basetest.c
augsys_basetest_LDADD = \
	$(top_srcdir)/src/c/augsys/libaugsys.la

augsys_errinfotest_SOURCES = errinfotest.c
augsys_errinfotest_LDADD = \
	$(top_srcdir)/src/c/augsys/libaugsys.la

augsys_sockettest_SOURCES = sockettest.c
augsys_sockettest_LDADD = \
	$(top_srcdir)/src/c/augsys/libaugsys.la
