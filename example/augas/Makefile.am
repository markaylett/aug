include $(top_srcdir)/src/cpp/lang.mk

EXTRA_DIST = \
	bench.conf \
	modules.mk

exampledir = $(prefix)/example/augas
example_DATA = \
	$(EXTRA_DIST) \
	client.cpp \
	http.cpp \
	minimal.c \
	server.cpp

moddir = $(prefix)/mod
mod_LTLIBRARIES = \
	modclient.la \
	modhttp.la \
	modminimal.la \
	modserver.la

modclient_la_SOURCES = client.cpp
modclient_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modclient_la_LIBADD = \
	$(top_builddir)/src/c/augsys/libaugsys.la

modhttp_la_SOURCES = http.cpp
modhttp_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modhttp_la_LIBADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la

modminimal_la_SOURCES = minimal.c
modminimal_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modminimal_la_LIBADD =

modserver_la_SOURCES = server.cpp
modserver_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modserver_la_LIBADD =
