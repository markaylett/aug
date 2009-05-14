include $(top_srcdir)/src/cpp/lang.mk

moddir = $(prefix)/mod

noinst_HEADERS =

mod_LTLIBRARIES = \
	modaugarc.la

modaugarc_la_SOURCES = \
	augarc.cpp
modaugarc_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modaugarc_la_LIBADD = \
	$(top_builddir)/src/cpp/augarcpp/libaugarcpp.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
