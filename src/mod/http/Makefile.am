include $(top_srcdir)/src/cpp/lang.mk

noinst_HEADERS = \
	file.hpp

moddir = $(prefix)/mod
mod_LTLIBRARIES = \
	modhttp.la

modhttp_la_SOURCES = \
	file.cpp \
	http.cpp
modhttp_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modhttp_la_LIBADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
