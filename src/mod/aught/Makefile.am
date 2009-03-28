include $(top_srcdir)/src/cpp/lang.mk

moddir = $(prefix)/mod

noinst_HEADERS = \
	exception.hpp \
	file.hpp \
	options.hpp

mod_LTLIBRARIES = \
	modaught.la

modaught_la_SOURCES = \
	exception.cpp \
	file.cpp \
	aught.cpp \
	options.cpp
modaught_la_LDFLAGS = -module -export-dynamic -avoid-version -no-undefined
modaught_la_LIBADD = \
	$(top_builddir)/src/c/augnet/libaugnet.la \
	$(top_builddir)/src/c/augutil/libaugutil.la \
	$(top_builddir)/src/c/augmar/libaugmar.la \
	$(top_builddir)/src/c/augsys/libaugsys.la \
	$(top_builddir)/src/c/augctx/libaugctx.la
