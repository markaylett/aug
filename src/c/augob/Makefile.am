EXTRA_DIST = \
	addrob.xml \
	blob.xml \
	channelob.xml \
	errinfob.xml \
	eventob.xml \
	iterob.xml \
	longob.xml \
	rpcob.xml \
	seqob.xml \
	streamob.xml

modincludedir = $(includedir)/augob
modinclude_HEADERS = \
	addrob.h \
	blob.h \
	channelob.h \
	errinfob.h \
	eventob.h \
	iterob.h \
	longob.h \
	rpcob.h \
	seqob.h \
	streamob.h \
	var.h

if ENABLE_RUBY
BUILT_SOURCES = \
	addrob.h \
	blob.h \
	channelob.h \
	errinfob.h \
	eventob.h \
	iterob.h \
	longob.h \
	rpcob.h \
	seqob.h \
	streamob.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
