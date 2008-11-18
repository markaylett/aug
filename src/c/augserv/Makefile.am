include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

modincludedir = $(includedir)/augserv
modinclude_HEADERS = \
	base.h \
	config.h \
	control.h \
	daemon.h \
	log.h \
	main.h \
	options.h \
	signal.h \
	types.h

lib_LTLIBRARIES = libaugserv.la

libaugserv_la_SOURCES = \
	base.c \
	config.c \
	control.c \
	daemon.c \
	log.c \
	main.c \
	options.c \
	signal.c \
	types.c

libaugserv_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugserv_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

check_PROGRAMS = maintest
TESTS = maintest

maintest_SOURCES = maintest.c
maintest_LDADD = \
	$(top_builddir)/src/c/augserv/libaugserv.la
