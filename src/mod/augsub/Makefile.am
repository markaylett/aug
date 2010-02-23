include $(top_srcdir)/src/cpp/lang.mk

moddir = $(prefix)/mod

noinst_HEADERS =

mod_LTLIBRARIES = \
	modaugsub.la

modaugsub_la_SOURCES = \
	augsub.cpp
modaugsub_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modaugsub_la_LIBADD = \
	$(top_builddir)/src/cpp/augsubpp/libaugsubpp.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
