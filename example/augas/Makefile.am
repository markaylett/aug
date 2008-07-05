include $(top_srcdir)/src/cpp/lang.mk

SUBDIRS = \
	etc \
	htdocs

EXTRA_DIST = \
	bench.conf \
	bench.R \
	bench.sh \
	gprof2dot.py \
	modules.mk \
	test.conf

exampledir = $(prefix)/example/augas
example_DATA = \
	$(EXTRA_DIST) \
	client.cpp \
	command.cpp \
	http.cpp \
	minimal.c \
	sched.cpp \
	server.cpp

moddir = $(prefix)/mod
mod_LTLIBRARIES = \
	modclient.la \
	modcommand.la \
	modhttp.la \
	modminimal.la \
	modsched.la \
	modserver.la

modclient_la_SOURCES = client.cpp
modclient_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modclient_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

modcommand_la_SOURCES = command.cpp
modcommand_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modcommand_la_LIBADD = \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

modhttp_la_SOURCES = http.cpp
modhttp_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modhttp_la_LIBADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

modminimal_la_SOURCES = minimal.c
modminimal_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modminimal_la_LIBADD =

modsched_la_SOURCES = sched.cpp
modsched_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modsched_la_LIBADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la

modserver_la_SOURCES = server.cpp
modserver_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modserver_la_LIBADD =
