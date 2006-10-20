include $(top_srcdir)/src/cpp/lang.mk

modincludedir = $(includedir)/augas
modinclude_HEADERS = \
	module.h

bin_PROGRAMS = augasd

augasd_SOURCES = \
	augasd.cpp

augasd_LDADD = \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la
