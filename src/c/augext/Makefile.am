EXTRA_DIST = \
	blob.xml \
	boxint.xml \
	boxptr.xml \
	chan.xml \
	clock.xml \
	ctx.xml \
	err.xml \
	http.xml \
	iter.xml \
	log.xml \
	mar.xml \
	mpool.xml \
	msg.xml \
	rpc.xml \
	seq.xml \
	stream.xml

modincludedir = $(includedir)/augext
modinclude_HEADERS = \
	blob.h \
	boxint.h \
	boxptr.h \
	chan.h \
	clock.h \
	ctx.h \
	err.h \
	http.h \
	iter.h \
	log.h \
	mar.h \
	mpool.h \
	msg.h \
	rpc.h \
	seq.h \
	stream.h \
	var.h

if ENABLE_RUBY
BUILT_SOURCES = \
	blob.h \
	boxint.h \
	boxptr.h \
	chan.h \
	clock.h \
	ctx.h \
	err.h \
	http.h \
	iter.h \
	log.h \
	mar.h \
	mpool.h \
	msg.h \
	rpc.h \
	seq.h \
	stream.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
