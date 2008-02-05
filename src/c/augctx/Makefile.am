include $(top_srcdir)/src/c/lang.mk
SUBDIRS = \
	posix \
	win32
EXTRA_DIST = \
	clock.xml \
	ctx.xml \
	log.xml \
	mpool.xml

modincludedir = $(includedir)/augctx
modinclude_HEADERS = \
	base.h \
	clock.h \
	ctx.h \
	defs.h \
	lock.h \
	log.h \
	mpool.h \
	types.h \
	utility.h

noinst_HEADERS = \
	malloc.h \
	tls_.h

lib_LTLIBRARIES = libaugctx.la

libaugctx_la_SOURCES = \
	base.c \
	clock.c \
	ctx.c \
	defs.c \
	lock.c \
	log.c \
	mpool.c \
	utility.c \
	malloc.c \
	tls_.c

libaugctx_la_LDFLAGS = \
	-no-undefined \
	-version-info 1:0:0

libaugctx_la_LIBADD =

check_PROGRAMS =

TESTS =

if ENABLE_RUBY
BUILT_SOURCES = \
	clock.h \
	ctx.h \
	log.h \
	mpool.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
