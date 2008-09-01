EXTRA_DIST = \
	blob.xml \
	boxint.xml \
	boxptr.xml \
	chan.xml \
	clock.xml \
	ctx.xml \
	err.xml \
	iter.xml \
	log.xml \
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
	iter.h \
	log.h \
	mpool.h  \
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
	iter.h \
	log.h \
	mpool.h  \
	msg.h \
	rpc.h \
	seq.h \
	stream.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
