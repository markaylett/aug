include $(top_srcdir)/src/cpp/lang.mk

if ENABLE_CPPBUILD

noinst_PROGRAMS = mplexd

mplexd_SOURCES = \
	mplexd.cpp

mplexd_LDADD = \
	$(top_srcdir)/src/cpp/augnetpp/libaugnetpp.la \
	$(top_srcdir)/src/cpp/augsrvpp/libaugsrvpp.la \
	$(top_srcdir)/src/cpp/augutilpp/libaugutilpp.la \
	$(top_srcdir)/src/cpp/augsyspp/libaugsyspp.la \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

endif