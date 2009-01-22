include $(top_srcdir)/src/c/lang.mk

moddir = $(prefix)/mod

if WITH_PYTHON
PY_CFLAGS=`$(SHELL) $(top_srcdir)/src/sh/pyflags -c`
PY_LDFLAGS=`$(SHELL) $(top_srcdir)/src/sh/pyflags -l`
AM_CFLAGS = $(PY_CFLAGS)

noinst_HEADERS = \
	host.h \
	object.h

mod_LTLIBRARIES = modaugpy.la
modaugpy_la_SOURCES = \
	augpy.c \
	host.c \
	object.c
modaugpy_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined $(PY_LDFLAGS)
modaugpy_la_LIBADD =
endif
