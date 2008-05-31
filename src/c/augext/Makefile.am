EXTRA_DIST = \
	blob.xml \
	chan.xml \
	clock.xml \
	ctx.xml \
	error.xml \
	intbox.xml \
	iter.xml \
	log.xml \
	mpool.xml \
	msg.xml \
	ptrbox.xml \
	rpc.xml \
	seq.xml \
	stream.xml

modincludedir = $(includedir)/augext
modinclude_HEADERS = \
	blob.h \
	chan.h \
	clock.h \
	ctx.h \
	error.h \
	intbox.h \
	iter.h \
	log.h \
	mpool.h  \
	msg.h \
	ptrbox.h \
	rpc.h \
	seq.h \
	stream.h \
	var.h

if ENABLE_RUBY
BUILT_SOURCES = \
	blob.h \
	chan.h \
	clock.h \
	ctx.h \
	error.h \
	intbox.h \
	iter.h \
	log.h \
	mpool.h  \
	msg.h \
	ptrbox.h \
	rpc.h \
	seq.h \
	stream.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
