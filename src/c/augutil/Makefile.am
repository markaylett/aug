EXTRA_DIST = \
	filetest.sh \
	filetest.stdin \
	filetest.stdout
	networdstest.sh \
	networdstest.stdin \
	networdstest.stdout \
	shelllexertest.sh \
	shellwordstest.sh \
	shellwordstest.stdin \
	shellwordstest.stdout

include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32

modincludedir = $(includedir)/augutil
modinclude_HEADERS = \
	config.h \
	conv.h \
	event.h \
	file.h \
	getopt.h \
	hires.h \
	lexer.h \
	list.h \
	log.h \
	md5.h \
	networds.h \
	object.h \
	path.h \
	pwd.h \
	shellwords.h \
	timer.h \
	tmspec.h \
	types.h \
	xstr.h

noinst_HEADERS =

lib_LTLIBRARIES = libaugutil.la

libaugutil_la_SOURCES = \
	config.c \
	conv.c \
	event.c \
	file.c \
	getopt.c \
	hires.c \
	lexer.c \
	list.c \
	log.c \
	md5.c \
	networds.c \
	object.c \
	path.c \
	pwd.c \
	shellwords.c \
	timer.c \
	tmspec.c \
	types.c \
	xstr.c

libaugutil_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugutil_la_LIBADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

check_PROGRAMS = \
	eventtest \
	filetest \
	hirestest \
	networdstest \
	shelllexertest \
	shellwordstest \
	xstrtest

TESTS = \
	eventtest \
	filetest.sh \
	hirestest \
	networdstest.sh \
	shelllexertest.sh \
	shellwordstest.sh \
	xstrtest

eventtest_SOURCES = eventtest.c
eventtest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la

filetest_SOURCES = filetest.c
filetest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la

hirestest_SOURCES = hirestest.c
hirestest_LDADD = \
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

xstrtest_SOURCES = xstrtest.c
xstrtest_LDADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la
