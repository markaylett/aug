include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32
EXTRA_DIST = \
	malloc.c \
	malloc.h \
	clock.xml \
	ctx.xml \
	log.xml \
	mpool.xml

modincludedir = $(includedir)/augctx
modinclude_HEADERS = \
	base.h \
	clock.h \
	config.h \
	ctx.h \
	defs.h \
	errno.h \
	errinfo.h \
	lock.h \
	log.h \
	mpool.h \
	string.h \
	utility.h

noinst_HEADERS = \
	tls_.h

lib_LTLIBRARIES = libaugctx.la

libaugctx_la_SOURCES = \
	base.c \
	clock.c \
	config.c \
	ctx.c \
	defs.c \
	errinfo.c \
	errno.c \
	lock.c \
	log.c \
	mpool.c \
	string.c \
	utility.c \
	tls_.c

libaugctx_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugctx_la_LIBADD =

check_PROGRAMS = \
	basetest \
	logtest

TESTS = \
	basetest \
	logtest

basetest_SOURCES = basetest.c
basetest_LDADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

logtest_SOURCES = logtest.c
logtest_LDADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

if ENABLE_RUBY
BUILT_SOURCES = \
	clock.h \
	ctx.h \
	log.h \
	mpool.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
