include $(top_srcdir)/src/c/lang.mk

bin_PROGRAMS = httpd

httpd_SOURCES = \
	message.c \
	message.h \
	httpd.c \
	state.c \
	state.h

httpd_LDADD = \
	$(top_srcdir)/src/c/augnet/libaugnet.la \
	$(top_srcdir)/src/c/augsrv/libaugsrv.la \
	$(top_srcdir)/src/c/augutil/libaugutil.la \
	$(top_srcdir)/src/c/augmar/libaugmar.la \
	$(top_srcdir)/src/c/augsys/libaugsys.la \
	$(EXTRA_LIBS)
