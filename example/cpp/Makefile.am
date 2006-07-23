include $(top_srcdir)/src/cpp/lang.mk

if ENABLE_CPPBUILD

noinst_PROGRAMS = mcastsend mplexd timerd

mcastsend_SOURCES = \
	mcastsend.cpp

mcastsend_LDADD = \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

mplexd_SOURCES = \
	mplexd.cpp

mplexd_LDADD = \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

timerd_SOURCES = \
	timerd.cpp

timerd_LDADD = \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la

endif