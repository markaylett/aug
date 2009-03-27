EXTRA_DIST = \
	array.xml \
	blob.xml \
	boxint.xml \
	boxptr.xml \
	chan.xml \
	clock.xml \
	ctx.xml \
	err.xml \
	http.xml \
	log.xml \
	mar.xml \
	mpool.xml \
	msg.xml \
	rpc.xml \
	stream.xml \
	task.xml

modincludedir = $(includedir)/augext
modinclude_HEADERS = \
	array.h \
	blob.h \
	boxint.h \
	boxptr.h \
	chan.h \
	clock.h \
	ctx.h \
	err.h \
	http.h \
	log.h \
	mar.h \
	mpool.h \
	msg.h \
	rpc.h \
	stream.h \
	task.h \
	var.h

if WITH_RUBY
BUILT_SOURCES = \
	array.h \
	blob.h \
	boxint.h \
	boxptr.h \
	chan.h \
	clock.h \
	ctx.h \
	err.h \
	http.h \
	log.h \
	mar.h \
	mpool.h \
	msg.h \
	rpc.h \
	stream.h \
	task.h

%.h: %.xml
	$(top_srcdir)/src/rb/augidl -n $@ <$< >$@
endif
