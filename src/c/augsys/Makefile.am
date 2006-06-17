include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

AM_CFLAGS = -D_MT
#AM_LDFLAGS = -Wl,-lws2_32

modincludedir = $(includedir)/augsys
modinclude_HEADERS = \
	base.h \
	config.h \
	debug.h \
	defs.h \
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

lib_LTLIBRARIES = libaugsys.la

libaugsys_la_SOURCES = \
	base.c \
	config.c \
	debug.c \
	defs.c \
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
	windows.c

libaugsys_la_LDFLAGS = \
	-no-undefined \
	-version-info 0:0:0

libaugsys_la_LIBADD =

check_PROGRAMS = augsys-test
TESTS = $(check_PROGRAMS)

augsys_test_SOURCES = test.c
augsys_test_LDADD = \
	$(top_srcdir)/src/c/augsys/libaugsys.la \
	$(EXTRALIBS)
