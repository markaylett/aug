include $(top_srcdir)/src/cpp/lang.mk

EXTRA_DIST = \
	augasd.conf \
	augasd.sh

noinst_HEADERS = \
	buffer.hpp \
	conn.hpp \
	exception.hpp \
	file.hpp \
	listener.hpp \
	manager.hpp \
	module.hpp \
	options.hpp \
	sess.hpp \
	utility.hpp

bin_PROGRAMS = augasd

augasd_SOURCES = \
	augasd.cpp \
	buffer.cpp \
	conn.cpp \
	exception.cpp \
	file.cpp \
	listener.cpp \
	manager.cpp \
	module.cpp \
	options.cpp \
	sess.cpp \
	utility.cpp

augasd_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

moddir = $(prefix)/mod

mod_LTLIBRARIES = modskel.la
modskel_la_SOURCES = modskel.c
modskel_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modskel_la_LIBADD =

if ENABLE_PYTHON
PY_CFLAGS=`$(SHELL) $(top_srcdir)/src/sh/pyflags -c`
PY_LDFLAGS=`$(SHELL) $(top_srcdir)/src/sh/pyflags -l`
AM_CFLAGS = $(PY_CFLAGS)

mod_LTLIBRARIES += modpython.la
modpython_la_SOURCES = modpython.c
modpython_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined $(PY_LDFLAGS)
modpython_la_LIBADD =
endif
