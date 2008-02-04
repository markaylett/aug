include $(top_srcdir)/src/c/lang.mk
EXTRA_DIST = \
	clock.xml \
	ctx.xml \
	log.xml \
	mpool.xml

modincludedir = $(includedir)/augctx
modinclude_HEADERS = \
	clock.h \
	ctx.h \
	defs.h \
	log.h \
	mpool.h \
	types.h \
	utility.h

noinst_HEADERS = \
	malloc.h

lib_LTLIBRARIES = libaugctx.la

libaugctx_la_SOURCES = \
	clock.c \
	ctx.c \
	defs.c \
	log.c \
	malloc.c \
	mpool.c \
	utility.c

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
