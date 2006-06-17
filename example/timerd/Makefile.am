include $(top_srcdir)/src/cpp/lang.mk

bin_PROGRAMS = timerd

timerd_SOURCES = \
	timerd.cpp

timerd_LDADD = \
	$(top_srcdir)/src/cpp/augsrvpp/libaugsrvpp.la \
	$(top_srcdir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la \
	$(EXTRALIBS)
