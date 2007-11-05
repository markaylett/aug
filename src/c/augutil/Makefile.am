EXTRA_DIST = \
	networdstest.in \
	networdstest.out \
	networdstest.sh \
	shelllexertest.sh \
	shellwordstest.in \
	shellwordstest.out \
	shellwordstest.sh

include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

modincludedir = $(includedir)/augutil
modinclude_HEADERS = \
	clock.h \
	config.h \
	conv.h \
	event.h \
	file.h \
	getopt.h \
	lexer.h \
	list.h \
	log.h \
	md5.h \
	networds.h \
	path.h \
	pwd.h \
	shellwords.h \
	timer.h \
	tmspec.h \
	types.h \
	var.h \
	xstr.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugutil.la

libaugutil_la_SOURCES = \
	clock.c \
	config.c \
	conv.c \
	event.c \
	file.c \
	getopt.c \
	lexer.c \
	list.c \
	log.c \
	md5.c \
	networds.c \
	path.c \
	pwd.c \
	shellwords.c \
	timer.c \
	tmspec.c \
	var.c \
	xstr.c

libaugutil_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugutil_la_LIBADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	clocktest \
	eventtest \
	networdstest \
	shelllexertest \
	shellwordstest

TESTS = \
	clocktest \
	eventtest \
	networdstest.sh \
	shelllexertest.sh \
	shellwordstest.sh

clocktest_SOURCES = clocktest.c
clocktest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la

eventtest_SOURCES = eventtest.c
eventtest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la

networdstest_SOURCES = networdstest.c
networdstest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la

shelllexertest_SOURCES = shelllexertest.c
shelllexertest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la

shellwordstest_SOURCES = shellwordstest.c
shellwordstest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la
