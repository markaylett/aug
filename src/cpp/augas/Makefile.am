include $(top_srcdir)/src/cpp/lang.mk

EXTRA_DIST = \
	augasd.conf \
	augasd.sh \
	test.sh

SUBDIRS = \
	etc

noinst_HEADERS = \
	buffer.hpp \
	clntconn.hpp \
	conn.hpp \
	exception.hpp \
	listener.hpp \
	manager.hpp \
	module.hpp \
	object.hpp \
	options.hpp \
	rwtimer.hpp \
	serv.hpp \
	servconn.hpp \
	utility.hpp

bin_PROGRAMS = augasd

augasd_SOURCES = \
	augasd.cpp \
	buffer.cpp \
	clntconn.cpp \
	conn.cpp \
	exception.cpp \
	listener.cpp \
	manager.cpp \
	module.cpp \
	object.cpp \
	options.cpp \
	rwtimer.cpp \
	servconn.cpp \
	serv.cpp \
	utility.cpp

augasd_LDADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augsrv/libaugsrv.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

moddir = $(prefix)/mod
mod_LTLIBRARIES = modskel.la modskelpp.la

modskel_la_SOURCES = modskel.c
modskel_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modskel_la_LIBADD =

modskelpp_la_SOURCES = modskelpp.cpp
modskelpp_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modskelpp_la_LIBADD =
