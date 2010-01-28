include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32
EXTRA_DIST = \
	malloc.c \
	malloc.h

modincludedir = $(includedir)/augctx
modinclude_HEADERS = \
	atomic.h \
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
	atomic.c \
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
	atomictest \
	basetest \
	errinfotest \
	logtest

TESTS = \
	atomictest \
	basetest \
	errinfotest \
	logtest

atomictest_SOURCES = atomictest.c

basetest_SOURCES = basetest.c
basetest_LDADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

errinfotest_SOURCES = errinfotest.c
errinfotest_LDADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

logtest_SOURCES = logtest.c
logtest_LDADD = \
	$(top_builddir)/src/c/augctx/libaugctx.la

if WITH_CWEB
BUILT_SOURCES = \
	atomic.h \
	atomic.c \
	atomictest.c

atomic.h atomic.c atomictest.c: $(top_srcdir)/doc/atomic.w
	ctangle $< - atomictest.c
	@sed -s 's/HAVE_AUGCTX_CONFIG_H 0/HAVE_AUGCTX_CONFIG_H 1/g' <atomic.h >tmp.$$$$ && mv tmp.$$$$ atomic.h
	@cat $(top_srcdir)/doc/notice.txt atomic.h | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ atomic.h
	@cat $(top_srcdir)/doc/notice.txt atomic.c | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ atomic.c
	@cat $(top_srcdir)/doc/notice.txt atomictest.c | sed -e 's/\s$$//g' >tmp.$$$$ && mv tmp.$$$$ atomictest.c
endif
